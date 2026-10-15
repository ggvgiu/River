#pragma once
#include "framework.h"
#include "RiverRaidGame.h"
#include <mmsystem.h>
#include <array>

// Small procedural soundtrack. All WinMM operations stay on the main thread.
class GameAudio
{
public:
    GameAudio() = default;
    ~GameAudio();
    GameAudio(const GameAudio&) = delete;
    GameAudio& operator=(const GameAudio&) = delete;
    void Initialize();
    void Pump(const RiverRaid::SoundState& state, bool enabled);
    void Shutdown();
private:
    struct Buffer { WAVEHDR header{}; std::array<short, 1024> samples{}; };
    HWAVEOUT output_ = nullptr;
    std::array<Buffer, 4> buffers_{};
    unsigned shots_ = 0;
    unsigned explosions_ = 0;
    int shotSamples_ = 0;
    int explosionSamples_ = 0;
    double enginePhase_ = 0;
    double effectPhase_ = 0;
    uint32_t noise_ = 1;
};
