#pragma once
#include "GameInput.h"
#include "RiverCourse.h"
#include <array>
#include <deque>

namespace RiverRaid
{
    enum class Phase { Ready, Flying, Crashing, GameOver };
    struct CheatSettings
    {
        bool infiniteLives = false;
        bool invincible = false;
    };
    struct Missile { float x = 0; float y = 0; bool active = false; };
    struct Player
    {
        int score = 0;
        int lives = 3;
        int difficulty = 1; // Original difficulty stage: 1..48, then 47/48 repeat.
        int segment = 1; // Unbounded progress: odd=river, even=mountains.
        int Level() const { return (segment - 1) / 2 + 1; }
        uint16_t seed = DefaultSeed;
        bool expert = false; // console difficulty A: missiles are not steerable
    };
    struct CourseSection
    {
        float start;
        RiverCourse course;
        int number = 1;
    };
    struct SoundState
    {
        bool engine = false;
        bool refueling = false;
        bool lowFuel = false;
        float throttle = 0.5f;
        unsigned shots = 0;
        unsigned explosions = 0;
    };

    // Pure gameplay; one call to Tick corresponds to an original NTSC frame.
    class RiverRaidGame
    {
    public:
        RiverRaidGame();
        void Reset();
        bool SetBaseSeed(uint16_t seed);
        uint16_t BaseSeed() const { return baseSeed_; }
        void SelectPlayers();
        void ToggleInfiniteLives() { cheats_.infiniteLives = !cheats_.infiniteLives; }
        void ToggleInvincibility() { cheats_.invincible = !cheats_.invincible; }
        const CheatSettings& Cheats() const { return cheats_; }
        void TogglePause() { paused_ = !paused_; }
        void ToggleColor() { monochrome_ = !monochrome_; }
        void ToggleDifficulty(int player) { players_.at(player).expert = !players_.at(player).expert; }
        void Tick(const std::array<PilotInput, 2>& input);

        Phase State() const { return phase_; }
        bool Paused() const { return paused_; }
        bool Monochrome() const { return monochrome_; }
        int PlayerCount() const { return playerCount_; }
        int ActivePlayer() const { return activePlayer_; }
        const std::array<Player, 2>& Players() const { return players_; }
        float JetX() const { return jetX_; }
        int Banking() const { return direction_; }
        float Scroll() const { return scroll_; }
        float Fuel() const { return float(fuel_) / 65535.0f; }
        int Frame() const { return tick_; }
        int CrashTicks() const { return crashTicks_; }
        bool Refueling() const { return sound_.refueling; }
        const Missile& Bullet() const { return missile_; }
        const std::deque<CourseSection>& Sections() const { return sections_; }
        RiverRow Row(float worldY) const;
        bool IsWater(float x, float worldY) const;
        const SoundState& Sound() const { return sound_; }
    private:
        friend struct GameTestAccess;
        void BeginTurn();
        void EnsureCourse();
        void UpdateEnemies();
        void UpdateMissile(bool fire);
        void CheckJetCollisions();
        void Crash();
        void AwardPoints(int points);
        void MoveJet(const PilotInput& input);
        bool JetHits(const RiverObject& object, float sectionStart) const;

        std::array<Player, 2> players_{};
        uint16_t baseSeed_ = DefaultSeed;
        CheatSettings cheats_;
        std::deque<CourseSection> sections_;
        int playerCount_ = 1;
        int activePlayer_ = 0;
        Phase phase_ = Phase::Ready;
        bool paused_ = false;
        bool monochrome_ = false;
        float jetX_ = 76;
        float scroll_ = -48;
        int fuel_ = 65535;
        int speedY_ = 128;
        int speedX_ = 0;
        int triggerX_ = 0;
        int triggerY_ = 0;
        int direction_ = 0;
        int tick_ = 0;
        int crashTicks_ = 0;
        uint8_t enemyRandom_ = 1;
        Missile missile_;
        SoundState sound_;
    };
}
