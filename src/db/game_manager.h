//
// Created by hy on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GAME_MANAGER_H
#define TC_SERVER_STEAM_GAME_MANAGER_H

#include <any>
#include <memory>

#include <sqlite_orm/sqlite_orm.h>
using namespace sqlite_orm;

#include "game.h"

namespace tc
{
    class Context;

    class GameManager {
    public:
        explicit GameManager(const std::shared_ptr<Context>& ctx);
        void Init();

        void SaveOrUpdateGame(const std::shared_ptr<Game>& game);
        std::shared_ptr<Game> GetGameByGameId(uint64_t gid);
        std::vector<std::shared_ptr<Game>> GetAllGames();
        void DeleteGameByGameId(uint64_t gid);
        void BatchSaveOrUpdateGames(const std::vector<std::shared_ptr<Game>>& games);

    private:
        auto GetStorageTypeValue();
        auto InitAppDatabase(const std::string& name);

    private:
        std::shared_ptr<Context> context_ = nullptr;
        std::any db_storage_;
    };
}

#endif //TC_SERVER_STEAM_GAME_MANAGER_H
