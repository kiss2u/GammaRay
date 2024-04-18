//
// Created by hy on 2024/3/1.
//

#include "app_server.h"

#include <memory>
#include "app.h"
#include "context.h"
#include "tc_common_new/log.h"
#include "ws_media_router.h"
#include "ws_ipc_router.h"
#include "tc_common_new/data.h"
#include "message_processor.h"
#include "http_handler.h"

static std::string kUrlMedia = "/media";
static std::string kUrlControl = "/control";
static std::string kUrlIpc = "/ipc";

static std::string kApiServerState = "/api/server/state";
static std::string kApiSdpRequest = "/api/sdp/request";

namespace tc
{

    struct aop_log {
        bool before(http::web_request &req, http::web_response &rep) {
            asio2::ignore_unused(rep);
            printf("aop_log before %s\n", req.method_string().data());
            return true;
        }

        bool after(std::shared_ptr<asio2::http_session> &session_ptr, http::web_request &req, http::web_response &rep) {
            ASIO2_ASSERT(asio2::get_current_caller<std::shared_ptr<asio2::http_session>>().get() == session_ptr.get());
            asio2::ignore_unused(session_ptr, req, rep);
            printf("aop_log after\n");
            return true;
        }
    };

    AppServer::AppServer(const std::shared_ptr<Application> &app) : Connection(app) {
        this->app_ = app;
        this->context_ = app->GetContext();
        http_handler_ = std::make_shared<HttpHandler>(this->app_);
    }

    void AppServer::Start() {
        http_server_ = std::make_shared<asio2::http_server>(*this->context_->GetAsio2IoPool());
        http_server_->support_websocket(true);
        ws_data_ = std::make_shared<WsData>(WsData{
            .vars_ = {
                {"app",  this->app_},
                {"proc", this->msg_processor_}
            }
        });
        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlMedia, http_server_);
        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlControl, http_server_);
        AddWebsocketRouter<std::shared_ptr<asio2::http_server>>(kUrlIpc, http_server_);

        AddHttpRouter<std::shared_ptr<asio2::http_server>>("/", http_server_);

        bool ret = http_server_->start(ip_, std::to_string(port_));
        LOGI("App server start result: {}", ret);
    }

    void AppServer::Exit() {

    }

    void AppServer::PostMediaMessage(const std::string &data) {
        media_routers_.ApplyAll([=](const auto &k, const auto &v) {
            v->PostBinaryMessage(data);
        });
    }

    void AppServer::PostControlMessage(const std::string &data) {

    }

    void AppServer::PostIpcMessage(const std::string& msg) {
        ipc_routers_.ApplyAll([=](const auto& k, const auto& v) {
            v->PostBinaryMessage(msg);
        });
    }

    int AppServer::GetConnectionPeerCount() {
        return (int)media_routers_.Size();
    }

    void AppServer::NotifyPeerConnected() {
        Connection::NotifyPeerConnected();
    }

    void AppServer::NotifyPeerDisconnected() {
        Connection::NotifyPeerDisconnected();
    }

    template<typename Server>
    void AppServer::AddWebsocketRouter(const std::string &path, const Server &s) {
        auto fn_get_socket_fd = [](std::shared_ptr<asio2::http_session> &sess_ptr) -> uint64_t {
            return (uint64_t)sess_ptr->socket().native_handle();
        };
        s->bind(path, websocket::listener<asio2::http_session>{}
            .on("message", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr, std::string_view data) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlMedia) {
                    media_routers_.VisitAll([=](auto k, auto &v) mutable {
                        v->OnMessage(sess_ptr, data);
                    });
                } else if (path == kUrlControl) {
                    control_routers_.VisitAll([=](auto k, auto &v) mutable {
                        v->OnMessage(sess_ptr, data);
                    });
                } else if (path == kUrlIpc) {
                    ipc_routers_.VisitAll([=](auto k, auto& v) mutable {
                        v->OnMessage(sess_ptr, data);
                    });
                }
            })
            .on("open", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                LOGI("App server {} open", path);
                // boost socket native_handle指向底层socket的fd
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                WsRouterPtr router = nullptr;
                if (path == kUrlMedia) {
                    router = WsMediaRouter::Make(ws_data_);
                    media_routers_.Insert(socket_fd, router);
                    NotifyMediaClientConnected();
                } else if (path == kUrlControl) {

                } else if (path == kUrlIpc) {
                    router = WsIpcRouter::Make(ws_data_);
                    ipc_routers_.Insert(socket_fd, router);
                }

                if (router) {
                    router->OnOpen(sess_ptr);
                }

                this->NotifyPeerConnected();
            })
            .on("close", [=, this](std::shared_ptr<asio2::http_session> &sess_ptr) {
                auto socket_fd = fn_get_socket_fd(sess_ptr);
                if (path == kUrlMedia) {
                    media_routers_.Remove(socket_fd);
                    NotifyMediaClientDisConnected();
                    LOGI("App server {} close, media router size: {}", path, media_routers_.Size());
                } else if (path == kUrlControl) {

                } else if (path == kUrlIpc) {
                    ipc_routers_.Remove(socket_fd);
                    LOGI("App server {} close, ipc router size: {}", path, media_routers_.Size());
                }

                this->NotifyPeerDisconnected();
            })
            .on_ping([=, this](auto &sess_ptr) {
                LOGI("App server {} ping", path);
            })
            .on_pong([=, this](auto &sess_ptr) {
                LOGI("App server {} pong", path);
            })
        );
    }

    template<typename Server>
    void AppServer::AddHttpRouter(const std::string &path, const Server &s) {
        s->bind<http::verb::get, http::verb::post>(path, [=, this](http::web_request &req, http::web_response &rep) {
            if (path == "/") {
                rep.fill_text("I'm working...");
            }
            else if (path == kApiServerState) {
                http_handler_->HandleServerState(req, rep);
            }
            else if (path == kApiSdpRequest) {
                http_handler_->HandleWebRtcSdpRequest(req, rep);
            }

        }, aop_log{}, http::enable_cache);
    }

    void AppServer::NotifyMediaClientConnected() {
        context_->SendAppMessage(MsgClientConnected {
            .client_size_ = static_cast<int>(media_routers_.Size()),
        });
    }

    void AppServer::NotifyMediaClientDisConnected() {
        context_->SendAppMessage(MsgClientDisconnected {
            .client_size_ = static_cast<int>(media_routers_.Size()),
        });
    }
}