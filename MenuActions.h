#pragma once
#include "framework.h"
#include "RiverRaidGame.h"

namespace MenuActions
{
    bool HandleCommand(HWND window, HINSTANCE instance, WORD command,
        RiverRaid::RiverRaidGame& game, bool& soundEnabled);
    void UpdateChecks(HWND window, const RiverRaid::RiverRaidGame& game, bool soundEnabled);
}
