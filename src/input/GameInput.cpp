#include "platform/framework.h"
#include "input/GameInput.h"
#include <Xinput.h>
#include <algorithm>
#include <cmath>

namespace
{
    bool Held(int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; }

    class KeyboardInput final : public InputSource
    {
        std::array<PilotInput, 2> Read() override
        {
            return {{
                {float(Held(VK_RIGHT)) - float(Held(VK_LEFT)), float(Held(VK_UP)) - float(Held(VK_DOWN)), Held(VK_SPACE)},
                {float(Held('D')) - float(Held('A')), float(Held('W')) - float(Held('S')), Held('F')}
            }};
        }
    };

    // Dynamic loading keeps keyboards usable even on a PC without XInput.
    class ControllerInput final : public InputSource
    {
        using GetState = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);
        HMODULE library_ = nullptr;
        GetState getState_ = nullptr;
    public:
        ControllerInput()
        {
            for (const wchar_t* name : {L"xinput1_4.dll", L"xinput1_3.dll", L"xinput9_1_0.dll"})
            {
                library_ = LoadLibraryW(name);
                if (library_) break;
            }
            if (library_) getState_ = reinterpret_cast<GetState>(GetProcAddress(library_, "XInputGetState"));
        }
        ~ControllerInput() override { if (library_) FreeLibrary(library_); }
        std::array<PilotInput, 2> Read() override
        {
            std::array<PilotInput, 2> result{};
            if (!getState_) return result;
            for (DWORD player = 0; player < 2; ++player)
            {
                XINPUT_STATE state{};
                if (getState_(player, &state) != ERROR_SUCCESS) continue;
                const auto& pad = state.Gamepad;
                auto axis = [](SHORT value) {
                    constexpr float deadZone = 0.24f;
                    float normalized = std::clamp(float(value) / 32767.0f, -1.0f, 1.0f);
                    return std::abs(normalized) <= deadZone ? 0.0f :
                        std::copysign((std::abs(normalized) - deadZone) / (1 - deadZone), normalized);
                };
                auto& input = result[player];
                input.horizontal = axis(pad.sThumbLX);
                input.throttle = axis(pad.sThumbLY);
                if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) input.horizontal = -1;
                if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) input.horizontal = 1;
                if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP) input.throttle = 1;
                if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) input.throttle = -1;
                input.fire = (pad.wButtons & XINPUT_GAMEPAD_A) || pad.bRightTrigger > 30;
            }
            return result;
        }
    };
}

GameInput::GameInput()
{
    AddSource(std::make_unique<KeyboardInput>());
    AddSource(std::make_unique<ControllerInput>());
}

void GameInput::AddSource(std::unique_ptr<InputSource> source)
{
    sources_.push_back(std::move(source));
}

std::array<PilotInput, 2> GameInput::Read(bool enabled)
{
    std::array<PilotInput, 2> result{};
    if (!enabled) return result;
    for (const auto& source : sources_)
    {
        const auto device = source->Read();
        for (size_t player = 0; player < result.size(); ++player)
        {
            auto& merged = result[player];
            if (std::abs(device[player].horizontal) > std::abs(merged.horizontal)) merged.horizontal = device[player].horizontal;
            if (std::abs(device[player].throttle) > std::abs(merged.throttle)) merged.throttle = device[player].throttle;
            merged.fire |= device[player].fire;
        }
    }
    return result;
}
