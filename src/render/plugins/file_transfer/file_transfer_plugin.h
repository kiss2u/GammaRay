//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_RTC_PLUGIN_H
#define GAMMARAY_RTC_PLUGIN_H
#include <memory>
#include "plugin_interface/gr_data_consumer_plugin.h"

namespace tc
{

    class Message;
    class FileTransmitMsgInterface;

    class FileTransferPlugin : public GrDataConsumerPlugin {
    public:
        std::string GetPluginId() override;
        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        virtual bool OnCreate(const GrPluginParam& param) override;

        void OnMessage(const std::string& msg) override;
        void OnMessage(const std::shared_ptr<tc::Message>& msg) override;

    private:

        std::shared_ptr<FileTransmitMsgInterface> file_trans_msg_interface_ = nullptr;
    };

}

extern "C" __declspec(dllexport) void* GetInstance();

#endif //GAMMARAY_UDP_PLUGIN_H
