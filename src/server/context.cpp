//
// Created by RGAA on 2023/12/18.
//

#include "context.h"
#include "tc_common_new/task_runtime.h"
#include "tc_common_new/message_notifier.h"
#include <QApplication>

namespace tc
{
    std::shared_ptr<Context> Context::Make() {
        return std::make_shared<Context>();
    }

    Context::Context() {
        msg_notifier_ = std::make_shared<MessageNotifier>();
        asio2_pool_ = std::make_shared<asio2::iopool>();
        asio2_pool_->start();
    }

    std::shared_ptr<MessageNotifier> Context::GetMessageNotifier() {
        return msg_notifier_;
    }

    std::shared_ptr<MessageListener> Context::CreateMessageListener() {
        return msg_notifier_->CreateListener();
    }

    void Context::PostTask(std::function<void()>&& task) {
        asio2_pool_->post(std::move(task));
    }

    std::shared_ptr<asio2::iopool> Context::GetAsio2IoPool() {
        return asio2_pool_;
    }

    std::string Context::GetCurrentExeFolder() {
        return QCoreApplication::applicationDirPath().toStdString();
    }

}