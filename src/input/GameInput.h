#pragma once
#include <array>
#include <memory>
#include <vector>

// Device-neutral controls. The simulation never reads Windows keys or controllers.
struct PilotInput
{
    float horizontal = 0;
    float throttle = 0;
    bool fire = false;
};

class InputSource
{
public:
    virtual ~InputSource() = default;
    virtual std::array<PilotInput, 2> Read() = 0;
};

class GameInput
{
public:
    GameInput();
    void AddSource(std::unique_ptr<InputSource> source);
    std::array<PilotInput, 2> Read(bool enabled);
private:
    std::vector<std::unique_ptr<InputSource>> sources_;
};
