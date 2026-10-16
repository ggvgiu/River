#pragma once
#include "rendering/visuals/AssetModel.h"

namespace RiverRaid::Visuals
{
    AssetModel MakePlayerPlane(bool banking, int player);
    AssetModel MakeShip();
    AssetModel MakeHelicopter();
    AssetModel MakeEnemyPlane();
    AssetModel MakeFuelDepot();
    AssetModel MakeHouse();
    AssetModel MakeBridge();
}
