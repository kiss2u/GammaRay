//
// Created by RGAA on 2024/4/10.
//

#ifndef TC_SERVER_STEAM_GAME_MANAGER_H
#define TC_SERVER_STEAM_GAME_MANAGER_H

#include <any>
#include <memory>
#include <sqlite_orm/sqlite_orm.h>
#include "db_game.h"

using namespace sqlite_orm;

namespace tc
{
    class GrContext;
    class GrDatabase;

    class DBGameOperator {
    public:
        explicit DBGameOperator(const std::shared_ptr<GrContext>& ctx, const std::shared_ptr<GrDatabase>& db);

        void SaveOrUpdateGame(const std::shared_ptr<TcDBGame>& game);
        std::shared_ptr<TcDBGame> GetGameByGameId(uint64_t gid);
        // game added by user
        std::shared_ptr<TcDBGame> GetGameByExePath(const std::string& path);
        std::vector<std::shared_ptr<TcDBGame>> GetAllGames();
        void DeleteGameByGameId(uint64_t gid);
        void BatchSaveOrUpdateGames(const std::vector<std::shared_ptr<TcDBGame>>& games);

    private:
        std::shared_ptr<GrContext> context_ = nullptr;
        std::shared_ptr<GrDatabase> db_ = nullptr;
    };
}

#endif //TC_SERVER_STEAM_GAME_MANAGER_H
