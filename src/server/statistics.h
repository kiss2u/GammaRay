//
// Created by RGAA on 2024/3/6.
//

#ifndef TC_APPLICATION_STATISTICS_H
#define TC_APPLICATION_STATISTICS_H

#include <string>
#include <cstdint>
#include <vector>

namespace tc
{

    constexpr auto kMaxStatCounts = 120;

    class Statistics {
    public:

        static Statistics* Instance() {
            static Statistics st;
            return &st;
        }

        void IncreaseRunningTime();
        void AppendVideoFrameBytes(int bytes);
        void AppendAudioFrameBytes(int bytes);
        void AppendEncodeTime(uint32_t time);
        void AppendFrameGap(uint32_t time);

    public:

        // unit: S
        int64_t running_time_{};
        int64_t video_frame_bytes_{};
        int64_t audio_frame_bytes_{};

        std::vector<uint32_t> encode_times_;
        std::vector<uint32_t> video_frame_gaps_;

    };

}

#endif //TC_APPLICATION_STATISTICS_H
