//
// Created by RGAA on 2024-04-11.
//

#ifndef TC_SERVER_STEAM_RNST_APP_H
#define TC_SERVER_STEAM_RNST_APP_H

#include "tab_base.h"

namespace tc
{

    class RnApp : public TabBase {
    public:
        RnApp(const std::shared_ptr<Context>& ctx, QWidget *parent);
        ~RnApp() = default;

        void OnTabShow() override;
        void OnTabHide() override;

    };

}

#endif //TC_SERVER_STEAM_ST_INPUT_H