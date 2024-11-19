//
// Created by hy on 19/11/2024.
//

#include "gr_stream_plugin.h"

namespace tc
{

    GrStreamPlugin::GrStreamPlugin() : GrPluginInterface() {
        plugin_type_ = GrPluginType::kStream;
    }

    GrStreamPlugin::~GrStreamPlugin() {

    }

    void GrStreamPlugin::OnVideoEncoderCreated(const GrPluginEncodedVideoType& type, int width, int height) {

    }

    void GrStreamPlugin::OnEncodedVideoFrame(const GrPluginEncodedVideoType& video_type,
                                                const std::shared_ptr<Data>& data,
                                                uint64_t frame_index,
                                                int frame_width,
                                                int frame_height,
                                                bool key) {

    }

    void GrStreamPlugin::OnEncodedVideoFrameInProtobufFormat(const std::string& msg) {

    }

    void GrStreamPlugin::OnRawVideoFrameSharedTexture(uint64_t handle) {

    }

    void GrStreamPlugin::OnRawVideoFrameRgba(const std::shared_ptr<Image>& image) {

    }

    void GrStreamPlugin::OnRawVideoFrameYuv(const std::shared_ptr<Image>& image) {

    }

    void GrStreamPlugin::OnAudioFormat(int samples, int channels, int bits) {

    }

    void GrStreamPlugin::OnRawAudioData(const std::shared_ptr<Data>& data) {

    }

    void GrStreamPlugin::OnSplitRawAudioData(const std::shared_ptr<Data>& left_ch_data, const std::shared_ptr<Data>& right_ch_data) {

    }

    void GrStreamPlugin::OnSplitFFTAudioData(const std::vector<double>& left_fft, const std::vector<double>& right_fft) {

    }
}