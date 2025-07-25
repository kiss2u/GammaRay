//
// Created by RGAA on 2024-02-05.
//

#include "app_timer.h"
#include "rd_context.h"
#include "app/app_messages.h"
#include "tc_common_new/log.h"

#include <vector>
#include <format>

namespace tc
{

    AppTimer::AppTimer(const std::shared_ptr<RdContext>& ctx) {
        context_ = ctx;
        timer_ = std::make_shared<asio2::timer>();
    }

    void AppTimer::StartTimers() {
        auto durations = std::vector<AppTimerDuration>{
            kTimerDuration1000,
            kTimerDuration2000,
            kTimerDuration5000,
            kTimerDuration500,
            kTimerDuration100,
            kTimerDuration16,
        };
        for (const auto& duration : durations) {
            auto timer_id = std::format("tid:{}", (int)duration);
            timer_->start_timer(timer_id, (int)duration, [=, this]() {
                this->NotifyTimeout(duration);
            });
        }
    }

    void AppTimer::NotifyTimeout(AppTimerDuration duration) {
        if (duration == AppTimerDuration::kTimerDuration1000) {
            context_->SendAppMessage(MsgTimer1000{});
        }
        else if (duration == AppTimerDuration::kTimerDuration2000) {
            context_->SendAppMessage(MsgTimer2000{});
        }
        else if (duration == AppTimerDuration::kTimerDuration5000) {
            context_->SendAppMessage(MsgTimer5000{});
        }
        else if (duration == AppTimerDuration::kTimerDuration500) {
            context_->SendAppMessage(MsgTimer500{});
        }
        else if (duration == AppTimerDuration::kTimerDuration100) {
            context_->SendAppMessage(MsgTimer100{});
        }
        else if (duration == AppTimerDuration::kTimerDuration16) {
            context_->SendAppMessage(MsgTimer16{});
        }
    }

}