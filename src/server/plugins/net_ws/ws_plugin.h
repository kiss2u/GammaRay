//
// Created by RGAA on 15/11/2024.
//

#ifndef GAMMARAY_UDP_PLUGIN_H
#define GAMMARAY_UDP_PLUGIN_H

#include "plugin_interface/gr_plugin_interface.h"

namespace tc
{

    class WsPlugin : public GrPluginInterface {
    public:

        std::string GetPluginName() override;
        std::string GetVersionName() override;
        uint32_t GetVersionCode() override;

        void On1Second() override;

    };

}

extern "C" __declspec(dllexport) void* GetInstance();

void* GetInstance() {
    static tc::WsPlugin plugin;
    return (void*)&plugin;
}


#endif //GAMMARAY_UDP_PLUGIN_H