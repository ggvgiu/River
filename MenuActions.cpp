#include "MenuActions.h"
#include "Resource.h"
#include "SeedDialog.h"
#include <bcrypt.h>
#include <cstdio>

#pragma comment(lib, "bcrypt.lib")

namespace
{
INT_PTR CALLBACK About(HWND dialog, UINT message, WPARAM wParam, LPARAM)
{
    if (message == WM_INITDIALOG) return TRUE;
    if (message == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
    {
        EndDialog(dialog, LOWORD(wParam));
        return TRUE;
    }
    return FALSE;
}

}

bool MenuActions::HandleCommand(HWND window, HINSTANCE instance, WORD command,
    RiverRaid::RiverRaidGame& game, bool& soundEnabled, RenderOptions& options)
{
    // Add a case here for each new menu command in Resource.h / River.rc.
    switch (command)
    {
    case IDM_VIEW_3D: options.valley3D = !options.valley3D; break;
    case IDM_CAMERA_LOW: options.cameraAngle = CameraAngle::Low; break;
    case IDM_CAMERA_MEDIUM: options.cameraAngle = CameraAngle::Medium; break;
    case IDM_CAMERA_HIGH: options.cameraAngle = CameraAngle::High; break;
    case IDM_RESET: game.Reset(); break;
    case IDM_SELECT: game.SelectPlayers(); break;
    case IDM_PAUSE: game.TogglePause(); break;
    case IDM_DIFFICULTY1: game.ToggleDifficulty(0); break;
    case IDM_DIFFICULTY2: game.ToggleDifficulty(1); break;
    case IDM_SOUND: soundEnabled = !soundEnabled; break;
    case IDM_COLOR: game.ToggleColor(); break;
    case IDM_INFINITE_LIVES: game.ToggleInfiniteLives(); break;
    case IDM_INVINCIBILITY: game.ToggleInvincibility(); break;
    case IDM_SEED_RANDOM:
        {
            uint16_t seed;
            do
            {
                if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(&seed), sizeof(seed), BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
                {
                    MessageBoxW(window, L"Windows could not generate a random seed. Try manual entry.", L"River", MB_OK | MB_ICONERROR);
                    return true;
                }
            } while (seed == 0 || seed == game.BaseSeed());
            game.SetBaseSeed(seed);
        }
        break;
    case IDM_SEED_MANUAL:
        {
            uint16_t seed = game.BaseSeed();
            if (RequestSeed(window, instance, seed)) game.SetBaseSeed(seed);
        }
        break;
    case IDM_SEED_DEFAULT: game.SetBaseSeed(RiverRaid::DefaultSeed); break;
    case IDM_CONTROLS:
        MessageBoxW(window,
            L"Player 1: Arrow keys to steer / change speed, Space to fire.\n"
            L"Player 2: WASD to steer / change speed, F to fire.\n\n"
            L"Two players take alternating turns after losing a plane.\n"
            L"Up accelerates; down slows down. The plane stays near the bottom.\n"
            L"Fly over fuel depots to refuel, or shoot them for 80 points.\n"
            L"Destroy bridges to pass them. Avoid banks, islands and enemies.\n\n"
            L"F1: Reset    F2: Select one / two players    F3: Pause\n"
            L"F4 / F5: Player 1 / 2 difficulty (direct missiles when checked)\n"
            L"F6: Sound    F7: Color / black and white    F10: This help\n\n"
            L"F8: Infinite lives    F9: Invincibility (including empty fuel)\n\n"
            L"XInput controllers 1 / 2: Left stick or D-pad; A or right trigger fires.\n"
            L"The game stops automatically while its window is inactive.\n\n"
            L"Original game: Carol Shaw / Activision, 1982.\n"
            L"Native port based on the supplied Jentzsch / Jernigan disassembly.",
            L"River Raid - Controls", MB_OK | MB_ICONINFORMATION);
        break;
    case IDM_ABOUT:
        DialogBoxW(instance, MAKEINTRESOURCEW(IDD_ABOUTBOX), window, About);
        return true;
    case IDM_EXIT:
        DestroyWindow(window);
        return true;
    default:
        return false;
    }
    UpdateChecks(window, game, soundEnabled, options);
    return true;
}

void MenuActions::UpdateChecks(HWND window, const RiverRaid::RiverRaidGame& game, bool soundEnabled, const RenderOptions& options)
{
    HMENU menu = GetMenu(window);
    auto check = [menu](UINT id, bool enabled) {
        CheckMenuItem(menu, id, MF_BYCOMMAND | (enabled ? MF_CHECKED : MF_UNCHECKED));
    };
    check(IDM_SELECT, game.PlayerCount() == 2);
    check(IDM_PAUSE, game.Paused());
    check(IDM_DIFFICULTY1, game.Players()[0].expert);
    check(IDM_DIFFICULTY2, game.Players()[1].expert);
    check(IDM_SOUND, soundEnabled);
    check(IDM_COLOR, game.Monochrome());
    check(IDM_VIEW_3D, options.valley3D);
    const UINT camera = options.cameraAngle == CameraAngle::Low ? IDM_CAMERA_LOW :
        options.cameraAngle == CameraAngle::High ? IDM_CAMERA_HIGH : IDM_CAMERA_MEDIUM;
    CheckMenuRadioItem(menu, IDM_CAMERA_LOW, IDM_CAMERA_HIGH, camera, MF_BYCOMMAND);
    check(IDM_INFINITE_LIVES, game.Cheats().infiniteLives);
    check(IDM_INVINCIBILITY, game.Cheats().invincible);
    check(IDM_SEED_DEFAULT, game.BaseSeed() == RiverRaid::DefaultSeed);
    wchar_t seedLabel[80];
    swprintf_s(seedLabel, L"Current: 0x%04X (%u)", unsigned(game.BaseSeed()), unsigned(game.BaseSeed()));
    ModifyMenuW(menu, IDM_SEED_CURRENT, MF_BYCOMMAND | MF_STRING | MF_GRAYED, IDM_SEED_CURRENT, seedLabel);
}
