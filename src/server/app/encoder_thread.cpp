//
// Created by RGAA on 2023-12-24.
//

#include "encoder_thread.h"

#include "context.h"
#include "tc_common_new/data.h"
#include "tc_common_new/image.h"
#include "tc_common_new/thread.h"
#include "tc_common_new/file.h"
#include "tc_common_new/log.h"
#include "tc_common_new/time_ext.h"
#include "tc_common_new/message_notifier.h"
#include "tc_encoder_new/video_encoder_factory.h"
#include "tc_encoder_new/video_encoder.h"
#include "tc_encoder_new/ffmpeg_video_encoder.h"
#include "tc_encoder_new/nvenc_video_encoder.h"
#include "settings/settings.h"
#include "app/app_messages.h"
#include "settings/settings.h"
#include "server/statistics.h"
#ifdef WIN32
#include <d3d11.h>
#include <wrl/client.h>
#include "tc_common_new/win32/d3d_render.h"
#include "tc_common_new/win32/d3d_debug_helper.h"
#include "plugins/plugin_manager.h"
#include "plugin_interface/gr_stream_plugin.h"
#include "plugin_interface/gr_encoder_plugin.h"
#include "video_frame_carrier.h"
#endif

#define DEBUG_FILE 0
#define DEBUG_SAVE_D3D11TEXTURE_TO_FILE 0

namespace tc
{
#if DEBUG_SAVE_D3D11TEXTURE_TO_FILE
    std::shared_ptr<D3DRender> g_render;
#endif
    std::shared_ptr<EncoderThread> EncoderThread::Make(const std::shared_ptr<Context>& ctx) {
        return std::make_shared<EncoderThread>(ctx);
    }

    EncoderThread::EncoderThread(const std::shared_ptr<Context>& ctx) {
        context_ = ctx;
        settings_ = Settings::Instance();
        plugin_manager_ = context_->GetPluginManager();
        enc_thread_ = Thread::Make("encoder_thread", 5);
        enc_thread_->Poll();

#if DEBUG_FILE
        debug_file_ = File::OpenForWriteB("debug_encoder.h264");
#endif

        msg_listener_ = context_->CreateMessageListener();
        msg_listener_->Listen<MsgInsertKeyFrame>([=, this](const MsgInsertKeyFrame& msg) {
            if (video_encoder_) {
                video_encoder_->InsertIDR();
            }
            // plugins: InsertIdr
            {
                plugin_manager_->VisitEncoderPlugins([=](GrEncoderPlugin* plugin) {
                    plugin->InsertIdr();
                });
            }
        });
    }

    void EncoderThread::Encode(const std::shared_ptr<Data>& data, int width, int height, uint64_t frame_index) {
        auto image = Image::Make(data, width, height, 0);
        this->Encode(image, frame_index);
    }

    void EncoderThread::Encode(const std::shared_ptr<Image>& image, uint64_t frame_index) {
        if (frame_width_ != image->width || frame_height_ != image->height || !video_encoder_) {
            auto settings = Settings::Instance();
            if (video_encoder_) {
                video_encoder_->Exit();
                video_encoder_.reset();
            }
            tc::EncoderConfig encoder_config;
            encoder_config.width = image->width;
            encoder_config.height = image->height;
            encoder_config.codec_type = settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264 ? tc::EVideoCodecType::kH264 : tc::EVideoCodecType::kHEVC;
            encoder_config.gop_size = -1;
            encoder_config.fps = 60;
            encoder_config.bitrate = settings->encoder_.bitrate_ * 1000000;
            EncoderFeature encoder_feature{-1, 0};
            video_encoder_ = VideoEncoderFactory::CreateEncoder(context_->GetMessageNotifier(),
                                                                encoder_feature,
                                                                settings_->encoder_.encoder_select_type_,
                                                                encoder_config,
                                                                ECreateEncoderName::kFFmpeg);
            if (!video_encoder_) {
                LOGE("Create encoder failed, width: {}, height: {}, select type : {}, encoder name: {}",
                     image->width, image->height, (int)settings->encoder_.encoder_select_type_, (int)settings->encoder_.encoder_name_);
                return;
            }

            static uint64_t write_buffer = 0;
            video_encoder_->RegisterEncodeCallback([=, this](const std::shared_ptr<Image>& frame, uint64_t frame_index, bool key) {
                LOGI("Encoded: frame size:{}, frame index: {}, key frame: {}", frame->data->Size(), frame_index, key);
                if (debug_file_) {
                    debug_file_->Write(write_buffer, frame->data);
                    write_buffer += frame->data->Size();
                }

                //
                MsgVideoFrameEncoded msg {
                    .frame_width_ = static_cast<uint32_t>(frame->width),
                    .frame_height_ = static_cast<uint32_t>(frame->height),
                    .frame_format_ = (uint32_t )settings->encoder_.encoder_format_,
                    .frame_index_ = frame_index,
                    .key_frame_ = key,
                    .image_ = frame,
                };
                context_->SendAppMessage(msg);
            });

            frame_width_ = image->width;
            frame_height_ = image->height;
        }

        enc_thread_->Post(SimpleThreadTask::Make([=, this]() {
            auto beg = TimeExt::GetCurrentTimestamp();
            video_encoder_->Encode(image, frame_index);
            auto end = TimeExt::GetCurrentTimestamp();
            auto diff = end - beg;
            Statistics::Instance()->AppendEncodeDuration(diff);
        }));
    }

    void EncoderThread::Encode(const CaptureVideoFrame& cap_video_msg) {
        auto settings = Settings::Instance();
        last_capture_video_frame_ = cap_video_msg;
        auto frame_index = cap_video_msg.frame_index_;
#if DEBUG_SAVE_D3D11TEXTURE_TO_FILE
        Microsoft::WRL::ComPtr<ID3D11Texture2D> shared_texture;
        if(g_render) {
            shared_texture = g_render->OpenSharedTexture(reinterpret_cast<HANDLE>(handle));
            if(!shared_texture) {
                g_render = nullptr;
            }
        }

        if(!g_render) {
            g_render = D3DRender::Create(reinterpret_cast<HANDLE>(handle), shared_texture.ReleaseAndGetAddressOf());
        }

        if(shared_texture) {
            CopyID3D11Texture2D(shared_texture);
        }
#endif
        if (frame_width_ != cap_video_msg.frame_width_ || frame_height_ != cap_video_msg.frame_height_
            || encoder_format_ != settings->encoder_.encoder_format_ || !video_encoder_) {
            if (video_encoder_) {
                video_encoder_->Exit();
                video_encoder_.reset();
            }
            tc::EncoderConfig encoder_config;
            if (settings_->encoder_.encode_res_type_ == Encoder::EncodeResolutionType::kOrigin) {
                encoder_config.width = cap_video_msg.frame_width_;
                encoder_config.height = cap_video_msg.frame_height_;
                encoder_config.encode_width = cap_video_msg.frame_width_;
                encoder_config.encode_height = cap_video_msg.frame_height_;
                encoder_config.frame_resize = false;
            } else {
                encoder_config.width = settings_->encoder_.encode_width_;
                encoder_config.height = settings_->encoder_.encode_height_;
                encoder_config.encode_width = settings_->encoder_.encode_width_;
                encoder_config.encode_height = settings_->encoder_.encode_height_;
                encoder_config.frame_resize = true;
            }
            encoder_config.codec_type = settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264 ? tc::EVideoCodecType::kH264 : tc::EVideoCodecType::kHEVC;
            encoder_config.enable_adaptive_quantization = true;
            encoder_config.gop_size = -1;
            encoder_config.quality_preset = 1;
            encoder_config.fps = 60;
            encoder_config.multi_pass = tc::ENvdiaEncMultiPass::kMultiPassDisabled;
            encoder_config.rate_control_mode = tc::ERateControlMode::kRateControlModeCbr;
            encoder_config.sample_desc_count = 1;
            encoder_config.supports_intra_refresh = true;
            encoder_config.texture_format = cap_video_msg.frame_format_;
            encoder_config.bitrate = settings->encoder_.bitrate_ * 1000000;
            EncoderFeature encoder_feature{cap_video_msg.adapter_uid_, 0};
            video_encoder_ = VideoEncoderFactory::CreateEncoder(context_->GetMessageNotifier(),
                                                                encoder_feature,
                                                                settings_->encoder_.encoder_select_type_,
                                                                encoder_config,
                                                                ECreateEncoderName::kNVENC);
            if (!video_encoder_) {
                printf("video_encoder_ create error\n");
                return;
            }

            // video frame carrier
            {
                if (frame_carrier_ != nullptr) {
                    frame_carrier_->Exit();
                    frame_carrier_.reset();
                }
                frame_carrier_ = std::make_shared<VideoFrameCarrier>(context_, cap_video_msg.adapter_uid_);
            }

            // plugins: Create encoder plugin
            {
                auto nvenc_encoder = plugin_manager_->GetNvencEncoderPlugin();
                if (nvenc_encoder && nvenc_encoder->Init(encoder_config)) {
                    working_encoder_plugin_ = nvenc_encoder;
                } else {
                    LOGW("Init NVENC failed, will try AMF.");
                    auto amf_encoder = plugin_manager_->GetAmfEncoderPlugin();
                    if (amf_encoder && amf_encoder->Init(encoder_config)) {
                        working_encoder_plugin_ = amf_encoder;
                    } else {
                        LOGW("Init AMF failed, will try FFmpeg.");
                        auto ffmpeg_encoder = plugin_manager_->GetFFmpegEncoderPlugin();
                        if (ffmpeg_encoder && ffmpeg_encoder->Init(encoder_config)) {
                            working_encoder_plugin_ = ffmpeg_encoder;
                        } else {
                            LOGE("Init FFmpeg failed, we can't encode frame in this machine!");
                            return;
                        }
                    }
                }
                LOGI("Finally, we use encoder plugin: {}, version: {}", working_encoder_plugin_->GetPluginName(), working_encoder_plugin_->GetVersionName());
            }

            auto video_type = [=]() -> GrPluginEncodedVideoType {
                if (settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kH264) {
                    return GrPluginEncodedVideoType::kH264;
                } else if (settings->encoder_.encoder_format_ == Encoder::EncoderFormat::kHEVC) {
                    return GrPluginEncodedVideoType::kH265;
                } else {
                    return GrPluginEncodedVideoType::kH264;
                }
            } ();

            // plugins: VideoEncoderCreated
            context_->PostStreamPluginTask([=, this]() {
                plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                    plugin->OnVideoEncoderCreated(video_type, encoder_config.width, encoder_config.height);
                });
            });

            static uint64_t write_buffer = 0;
            video_encoder_->RegisterEncodeCallback([=, this](const std::shared_ptr<Image>& frame, uint64_t frame_index, bool key) {
                if (key) {
                    LOGI("Encoded: frame size:{}, frame index: {}, key frame: {}, size: {}x{}, monitor: {} - {} - ({},{}, {},{})",
                         frame->data->Size(), frame_index, key, frame->width, frame->height, last_capture_video_frame_.monitor_index_, last_capture_video_frame_.display_name_,
                         last_capture_video_frame_.left_, last_capture_video_frame_.top_, last_capture_video_frame_.right_, last_capture_video_frame_.bottom_);
                }

                MsgVideoFrameEncoded msg {
                    .frame_width_ = static_cast<uint32_t>(frame->width),
                    .frame_height_ = static_cast<uint32_t>(frame->height),
                    .frame_format_ = (uint32_t)settings->encoder_.encoder_format_,
                    .frame_index_ = frame_index,
                    .key_frame_ = key,
                    .image_ = frame,
                    .monitor_index_ = last_capture_video_frame_.monitor_index_,
                    .monitor_name_ = last_capture_video_frame_.display_name_,
                    .monitor_left_ = last_capture_video_frame_.left_,
                    .monitor_top_ = last_capture_video_frame_.top_,
                    .monitor_right_ = last_capture_video_frame_.right_,
                    .monitor_bottom_ = last_capture_video_frame_.bottom_,
                };
                context_->SendAppMessage(msg);

            });

            frame_width_ = cap_video_msg.frame_width_;
            frame_height_ = cap_video_msg.frame_height_;
            encoder_format_ = settings->encoder_.encoder_format_;
        }

        PostEncTask([=, this]() {
            // copy shared texture
            if (frame_carrier_ == nullptr) {
                LOGI("Don't have frame carrier !");
                return;
            }
            auto beg = TimeExt::GetCurrentTimestamp();
            auto target_texture = frame_carrier_->CopyTexture(cap_video_msg.handle_, frame_index);
            if (target_texture == nullptr) {
                LOGI("Don't have target texture, frame carrier copies texture failed!");
                return;
            }

            video_encoder_->Encode(target_texture);
            bool can_encode_texture = working_encoder_plugin_ && working_encoder_plugin_->CanEncodeTexture();
            if (can_encode_texture) {
                working_encoder_plugin_->Encode(target_texture, frame_index);
            }
            auto end = TimeExt::GetCurrentTimestamp();
            auto diff = end - beg;
            Statistics::Instance()->AppendEncodeDuration(diff);

            // TODO: May make latency !!!
            D3D11_TEXTURE2D_DESC desc;
            target_texture->GetDesc(&desc);
            auto rgba_cbk = [=, this](const std::shared_ptr<Image>& image) {
                // callback in Enc thread
                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                        plugin->OnRawVideoFrameRgba(image);
                    });
                });
            };
            auto yuv_cbk = [=, this](const std::shared_ptr<Image>& image) {
                // callback in YUV converter thread
                if (working_encoder_plugin_) {
                    PostEncTask([=, this]() {
                        working_encoder_plugin_->Encode(image, frame_index);
                    });
                }
                context_->PostStreamPluginTask([=, this]() {
                    plugin_manager_->VisitStreamPlugins([=, this](GrStreamPlugin *plugin) {
                        plugin->OnRawVideoFrameYuv(image);
                    });
                });
            };
            frame_carrier_->MapRawTexture(target_texture, desc.Format, (int)desc.Height, rgba_cbk, yuv_cbk);
        });
    }

    void EncoderThread::Exit() {
        if (enc_thread_) {
            enc_thread_->Exit();
        }
    }

    void EncoderThread::PostEncTask(std::function<void()>&& task) {
        enc_thread_->Post(std::move(task));
    }

}