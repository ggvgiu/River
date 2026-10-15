#pragma once
#include "framework.h"
#include "RiverRaidGame.h"
#include "RenderOptions.h"

namespace MenuActions
{
    bool HandleCommand(HWND window, HINSTANCE instance, WORD command,
        RiverRaid::RiverRaidGame& game, bool& soundEnabled, RenderOptions& options);
    void UpdateChecks(HWND window, const RiverRaid::RiverRaidGame& game, bool soundEnabled, const RenderOptions& options);
}
