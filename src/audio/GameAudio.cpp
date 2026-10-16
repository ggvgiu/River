#include "audio/GameAudio.h"
#include <algorithm>
#include <cmath>

#pragma comment(lib, "winmm.lib")

GameAudio::~GameAudio() { Shutdown(); }

void GameAudio::Initialize()
{
    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 1;
    format.nSamplesPerSec = 44100;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 2;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    if (waveOutOpen(&output_, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
    { output_ = nullptr; return; }
    for (auto& buffer : buffers_)
    {
        buffer.header.lpData = reinterpret_cast<LPSTR>(buffer.samples.data());
        buffer.header.dwBufferLength = DWORD(buffer.samples.size() * sizeof(short));
        if (waveOutPrepareHeader(output_, &buffer.header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        { Shutdown(); return; }
    }
}

void GameAudio::Shutdown()
{
    if (!output_) return;
    waveOutReset(output_);
    for (auto& buffer : buffers_)
        if (buffer.header.dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(output_, &buffer.header, sizeof(WAVEHDR));
    waveOutClose(output_);
    output_ = nullptr;
}

void GameAudio::Pump(const RiverRaid::SoundState& state, bool enabled)
{
    if (!output_) return;
    if (enabled && state.shots > shots_) shotSamples_ = 6000;
    if (enabled && state.explosions > explosions_) explosionSamples_ = 18000;
    shots_ = state.shots;
    explosions_ = state.explosions;
    if (!enabled) shotSamples_ = explosionSamples_ = 0;
    for (auto& buffer : buffers_)
    {
        if ((buffer.header.dwFlags & WHDR_INQUEUE) != 0) continue;
        for (auto& sample : buffer.samples)
        {
            double value = 0;
            enginePhase_ += (65 + state.throttle * 70) / 44100.0;
            enginePhase_ -= std::floor(enginePhase_);
            if (enabled && state.engine) value += (enginePhase_ < 0.5 ? 0.045 : -0.045);
            effectPhase_ += (state.refueling ? 650 : 980) / 44100.0;
            effectPhase_ -= std::floor(effectPhase_);
            if (enabled && state.refueling) value += (effectPhase_ < 0.5 ? 0.06 : -0.06);
            if (enabled && state.lowFuel && state.engine && enginePhase_ < 0.15) value += 0.05;
            noise_ = noise_ * 1664525u + 1013904223u;
            double noise = double((noise_ >> 16) & 65535) / 32767.5 - 1;
            if (shotSamples_ > 0) { value += noise * 0.10 * shotSamples_ / 6000; --shotSamples_; }
            if (explosionSamples_ > 0) { value += noise * 0.23 * explosionSamples_ / 18000; --explosionSamples_; }
            sample = short(std::clamp(value, -1.0, 1.0) * 32767);
        }
        waveOutWrite(output_, &buffer.header, sizeof(WAVEHDR));
    }
}
