#include "AssetLibrary.h"
#include "models/BuiltinModels.h"
#include <utility>

namespace RiverRaid::Visuals
{
    AssetLibrary::AssetLibrary()
    {
        SetObject(ObjectKind::Ship, MakeShip());
        SetObject(ObjectKind::Helicopter, MakeHelicopter());
        SetObject(ObjectKind::Plane, MakeEnemyPlane());
        SetObject(ObjectKind::Fuel, MakeFuelDepot());
        SetObject(ObjectKind::House, MakeHouse());
        SetObject(ObjectKind::Bridge, MakeBridge());
        for (int player = 0; player < 2; ++player)
            for (bool banking : {false, true}) SetPlayer(banking, player, MakePlayerPlane(banking, player));
    }

    const AssetModel& AssetLibrary::Object(ObjectKind kind) const { return objects_.at(size_t(kind)); }
    const AssetModel& AssetLibrary::Player(bool banking, int player) const { return players_.at(player).at(banking ? 1 : 0); }
    void AssetLibrary::SetObject(ObjectKind kind, AssetModel model) { objects_.at(size_t(kind)) = std::move(model); }
    void AssetLibrary::SetPlayer(bool banking, int player, AssetModel model) { players_.at(player).at(banking ? 1 : 0) = std::move(model); }
}
