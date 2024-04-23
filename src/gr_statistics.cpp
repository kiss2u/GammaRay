//
// Created by RGAA on 2024-04-20.
//

#include "gr_statistics.h"

#include "app_messages.h"
#include "gr_context.h"
#include "tc_common_new/log.h"

namespace tc
{

    void GrStatistics::RegisterEventListeners() {
        msg_listener_ = context_->GetMessageNotifier()->CreateListener();
        msg_listener_->Listen<MsgCaptureStatistics>([=, this](const MsgCaptureStatistics& msg) {
            this->video_frame_gaps_.clear();
            this->video_frame_gaps_.insert(this->video_frame_gaps_.begin(),
                                           msg.statistics_.video_frame_gaps().begin(),
                                           msg.statistics_.video_frame_gaps().end());
            this->encode_durations_.clear();
            this->encode_durations_.insert(this->encode_durations_.begin(),
                                           msg.statistics_.encode_durations().begin(),
                                           msg.statistics_.encode_durations().end());
            this->audio_frame_gaps_.clear();
            this->audio_frame_gaps_.insert(this->audio_frame_gaps_.begin(),
                                           msg.statistics_.audio_frame_gaps().begin(),
                                           msg.statistics_.audio_frame_gaps().end());
            this->decode_durations_.clear();
            this->decode_durations_.insert(this->decode_durations_.begin(),
                                           msg.statistics_.decode_durations().begin(),
                                           msg.statistics_.decode_durations().end());
            this->client_video_recv_gaps_.clear();
            this->client_video_recv_gaps_.insert(this->client_video_recv_gaps_.begin(),
                                           msg.statistics_.client_video_recv_gaps().begin(),
                                           msg.statistics_.client_video_recv_gaps().end());
            this->client_fps_video_recv_ = msg.statistics_.client_fps_video_recv();
            this->client_fps_render_ = msg.statistics_.client_fps_render();
            this->client_recv_media_data_ = msg.statistics_.client_recv_media_data();
            // from inner server
            this->fps_video_encode = msg.statistics_.fps_video_encode();
            // from inner server
            this->app_running_time = msg.statistics_.app_running_time();
            // from inner server
            this->server_send_media_bytes = msg.statistics_.server_send_media_data();

        });
    }

}
