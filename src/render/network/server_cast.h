//
// Created by RGAA on 2024/3/15.
//

#ifndef TC_APPLICATION_SERVER_CAST_H
#define TC_APPLICATION_SERVER_CAST_H

#include <memory>

namespace tc
{

    class RdContext;

    class ServerCast {
    public:

        static std::shared_ptr<ServerCast> Make(std::shared_ptr<RdContext>& ctx);

        explicit ServerCast(const std::shared_ptr<RdContext>& ctx);
        void Start();
        void Stop();

    private:
        std::shared_ptr<RdContext> context_ = nullptr;

    };

}

#endif //TC_APPLICATION_SERVER_CAST_H
