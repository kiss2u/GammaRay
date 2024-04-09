//
// Created by hy on 2024/1/17.
//

#include "context.h"

#include "tc_common_new/task_runtime.h"
#include "tc_common_new/shared_preference.h"
#include "tc_common_new/uuid.h"
#include "tc_steam_manager_new/steam_manager.h"
#include "tc_common_new/log.h"

namespace tc
{

    constexpr auto kKeySysUniqueId = "sys_unique_id";

    Context::Context() : QObject(nullptr) {

    }

    void Context::Init() {
        sp_ = SharedPreference::Instance();
        sp_->Init("", "tc_steam.dat");
        // unique id
        LoadUniqueId();

        task_runtime_ = std::make_shared<TaskRuntime>();
        steam_mgr_ = SteamManager::Make(task_runtime_);

        // ips
        ips_ = IPUtil::ScanIPs();

        LOGI("Unique Id: {}", unique_id_);
        LOGI("Scan IP size: {}", ips_.size());
        for (auto& [ip, type] : ips_) {
            LOGI("IP: {} -> {}", ip, type == IPNetworkType::kWired ? "Wired" : "Wireless");
        }
    }

    std::shared_ptr<SteamManager> Context::GetSteamManager() {
        return steam_mgr_;
    }

    std::shared_ptr<TaskRuntime> Context::GetTaskRuntime() {
        return task_runtime_;
    }

    void Context::PostTask(std::function<void()>&& task) {
        task_runtime_->Post(SimpleThreadTask::Make(std::move(task)));
    }

    void Context::PostUITask(std::function<void()>&& task) {
        QMetaObject::invokeMethod(this, [=]() {
            task();
        });
    }

    void Context::LoadUniqueId() {
        unique_id_ = sp_->Get(kKeySysUniqueId);
        if (unique_id_.empty()) {
            GenUniqueId();
            sp_->Put(kKeySysUniqueId, unique_id_);
        }
    }

    void Context::GenUniqueId() {
        auto uuid = GetUUID();
        std::hash<std::string> fn_hash;
        size_t value = fn_hash(uuid);
        unique_id_ = std::to_string(value%1000000);
    }

    std::string Context::GetSysUniqueId() {
        return unique_id_;
    }

    std::map<std::string, IPNetworkType> Context::GetIps() {
        return ips_;
    }

}