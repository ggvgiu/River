#include "RiverRaidGame.h"
#include "SpriteAssets.h"
#include <algorithm>
#include <cmath>

namespace RiverRaid
{
    RiverRaidGame::RiverRaidGame() { Reset(); }

    bool RiverRaidGame::SetBaseSeed(uint16_t seed)
    {
        // Zero is the absorbing state of the original LFSR.
        if (seed == 0) return false;
        baseSeed_ = seed;
        Reset();
        return true;
    }

    void RiverRaidGame::Reset()
    {
        const bool expert1 = players_[0].expert;
        const bool expert2 = players_[1].expert;
        players_ = {};
        for (auto& player : players_) player.seed = baseSeed_;
        players_[0].expert = expert1;
        players_[1].expert = expert2;
        activePlayer_ = 0;
        paused_ = false;
        tick_ = 0;
        enemyRandom_ = 1;
        sound_ = {};
        BeginTurn();
    }

    void RiverRaidGame::SelectPlayers()
    {
        playerCount_ = playerCount_ == 1 ? 2 : 1;
        Reset();
    }

    void RiverRaidGame::BeginTurn()
    {
        sections_.clear();
        const auto& player = players_[activePlayer_];
        sections_.push_back({0, RiverCourse(player.difficulty, player.seed), player.segment});
        scroll_ = -48;
        jetX_ = 76;
        fuel_ = 65535;
        speedY_ = 128;
        speedX_ = triggerX_ = triggerY_ = direction_ = 0;
        missile_ = {};
        crashTicks_ = 0;
        phase_ = Phase::Ready;
        sound_.engine = sound_.refueling = sound_.lowFuel = false;
        EnsureCourse();
    }

    void RiverRaidGame::EnsureCourse()
    {
        while (sections_.back().start + LevelLength < scroll_ + FieldHeight + 64)
        {
            const auto& last = sections_.back();
            int nextLevel = last.course.Difficulty() == 48 ? 47 : last.course.Difficulty() + 1;
            sections_.push_back({last.start + LevelLength, RiverCourse(nextLevel, last.course.NextSeed()), last.number + 1});
        }
        while (sections_.size() > 2 && sections_[1].start < scroll_ - 64) sections_.pop_front();
    }

    RiverRow RiverRaidGame::Row(float worldY) const
    {
        for (auto it = sections_.rbegin(); it != sections_.rend(); ++it)
            if (worldY >= it->start) return it->course.Row(worldY - it->start);
        return sections_.front().course.Row(worldY - sections_.front().start);
    }

    bool RiverRaidGame::IsWater(float x, float worldY) const
    {
        const auto row = Row(worldY);
        return x >= row.bank && x < FieldWidth - row.bank &&
            (row.island <= 0 || x < 80 - row.island || x >= 80 + row.island);
    }

    void RiverRaidGame::MoveJet(const PilotInput& input)
    {
        const int previousDirection = direction_;
        direction_ = input.horizontal > 0.15f ? 1 : input.horizontal < -0.15f ? -1 : 0;
        if (!direction_) speedX_ = triggerX_ = 0;
        else
        {
            // Native form of speedX/triggerX: +8 each frame, one pixel on overflow.
            speedX_ = std::min(255, speedX_ + 8);
            if (previousDirection != direction_) triggerX_ = direction_ > 0 ? 255 : 0;
            int sum = triggerX_ + direction_ * speedX_;
            if (sum < 0 || sum > 255 || speedX_ >= 248) jetX_ += float(direction_);
            triggerX_ = sum & 255;
        }
        jetX_ = std::clamp(jetX_, 0.0f, 152.0f);
        if (input.throttle > 0.15f) speedY_ = std::min(254, speedY_ + 2);
        else if (input.throttle < -0.15f) speedY_ = std::max(64, speedY_ - 2);
        else if (speedY_ < 128) speedY_ += 2;
        else if (speedY_ > 128) speedY_ -= 2;
    }

    void RiverRaidGame::UpdateEnemies()
    {
        const unsigned feedback = ((enemyRandom_ >> 7) ^ (enemyRandom_ >> 4)) & 1;
        enemyRandom_ = uint8_t((enemyRandom_ << 1) | feedback);
        for (auto& section : sections_)
        for (auto& object : section.course.Objects())
        {
            if (object.explosionTicks > 0) --object.explosionTicks;
            const float screenY = section.start + object.distance - scroll_;
            if (object.destroyed || screenY < -32 || screenY > FieldHeight + 32) continue;
            if (object.kind != ObjectKind::Ship && object.kind != ObjectKind::Helicopter && object.kind != ObjectKind::Plane) continue;
            if ((tick_ & 15) == 0 && ((int(object.distance) / 32) % 5) == (enemyRandom_ & 7) % 5)
                object.moving = true;
            if (object.kind != ObjectKind::Plane && (!object.moving || (tick_ & 1))) continue;
            float nextX = object.x + object.direction;
            const auto sprite = ObjectSprite(object.kind, tick_);
            if (object.kind == ObjectKind::Plane)
            {
                if (nextX > 159) nextX = 0;
                if (nextX < 0) nextX = 159;
            }
            else
            {
                bool fits = true;
                for (int y = 0; y < sprite.Height() && fits; ++y)
                for (int x = 0; x < sprite.Width(); ++x)
                    if (sprite.Solid(x, y, object.direction < 0) &&
                        !IsWater(nextX + x, section.start + object.distance + y)) { fits = false; break; }
                if (!fits) { object.direction = -object.direction; continue; }
            }
            object.x = nextX;
        }
    }

    void RiverRaidGame::AwardPoints(int points)
    {
        auto& player = players_[activePlayer_];
        int previous = player.score;
        player.score = std::min(999999, player.score + points);
        if (player.score / 10000 > previous / 10000) player.lives = std::min(9, player.lives + 1);
        if (player.score >= 999999) phase_ = Phase::GameOver;
    }

    void RiverRaidGame::UpdateMissile(bool fire)
    {
        if (!missile_.active)
        {
            if (!fire) return;
            missile_ = {jetX_ + 5, 13, true};
            ++sound_.shots;
        }
        else
        {
            if (!players_[activePlayer_].expert) missile_.x = jetX_ + 5;
            // Six screen lines per NTSC frame, as MISSILE_SPEED in the source.
            for (int step = 0; step < 6 && missile_.active; ++step)
            {
                ++missile_.y;
                if (missile_.y > FieldHeight) { missile_.active = false; break; }
                const float worldY = scroll_ + missile_.y;
                if (!IsWater(missile_.x, worldY)) { missile_.active = false; break; }
                for (auto& section : sections_)
                for (auto& object : section.course.Objects())
                {
                    if (!missile_.active || object.destroyed || object.kind == ObjectKind::House) continue;
                    const auto sprite = ObjectSprite(object.kind, tick_);
                    const int x = int(missile_.x - object.x);
                    const int y = int(worldY - section.start - object.distance);
                    const bool mirrored = object.kind != ObjectKind::Fuel && object.direction < 0;
                    if (sprite.Solid(x, y, mirrored) || sprite.Solid(x, y + 1, mirrored))
                    {
                        object.destroyed = true;
                        object.explosionTicks = object.kind == ObjectKind::Bridge ? 48 : 24;
                        missile_.active = false;
                        AwardPoints(ObjectScore(object.kind));
                        ++sound_.explosions;
                    }
                }
            }
        }
    }

    bool RiverRaidGame::JetHits(const RiverObject& object, float sectionStart) const
    {
        const auto jet = JetSprite(direction_ != 0);
        const auto other = ObjectSprite(object.kind, tick_);
        const float objectY = sectionStart + object.distance - scroll_;
        if (objectY > JetBottom + jet.Height() || objectY + other.Height() < JetBottom) return false;
        for (int y = 0; y < jet.Height(); ++y)
        for (int x = 0; x < jet.Width(); ++x)
            if (jet.Solid(x, y, direction_ < 0) && other.Solid(int(jetX_ + x - object.x),
                int(JetBottom + y - objectY), object.kind != ObjectKind::Fuel && object.direction < 0)) return true;
        return false;
    }

    void RiverRaidGame::Crash()
    {
        if (phase_ != Phase::Flying || cheats_.invincible) return;
        phase_ = Phase::Crashing;
        crashTicks_ = 90;
        if (!cheats_.infiniteLives) --players_[activePlayer_].lives;
        missile_.active = false;
        sound_.engine = sound_.refueling = false;
        ++sound_.explosions;
    }

    void RiverRaidGame::CheckJetCollisions()
    {
        const auto jet = JetSprite(direction_ != 0);
        for (int y = 0; y < jet.Height(); ++y)
        for (int x = 0; x < jet.Width(); ++x)
            if (!cheats_.invincible && jet.Solid(x, y, direction_ < 0) && !IsWater(jetX_ + x, scroll_ + JetBottom + y))
            { Crash(); return; }
        for (const auto& section : sections_)
        for (const auto& object : section.course.Objects())
        {
            if (object.destroyed || object.kind == ObjectKind::House || !JetHits(object, section.start)) continue;
            if (object.kind == ObjectKind::Fuel)
            {
                fuel_ = std::min(65535, fuel_ + 256);
                sound_.refueling = true;
            }
            else if (!cheats_.invincible) { Crash(); return; }
        }
    }

    void RiverRaidGame::Tick(const std::array<PilotInput, 2>& inputs)
    {
        if (paused_) return;
        ++tick_;
        sound_.refueling = false;
        if (phase_ == Phase::GameOver) return;
        const auto& input = inputs[activePlayer_];
        if (phase_ == Phase::Ready)
        {
            if (input.fire || input.horizontal != 0 || input.throttle != 0) phase_ = Phase::Flying;
            else return;
        }
        if (phase_ == Phase::Crashing)
        {
            if (--crashTicks_ > 0) return;
            int other = 1 - activePlayer_;
            if (playerCount_ == 2 && players_[other].lives > 0) activePlayer_ = other;
            if (players_[activePlayer_].lives <= 0) { phase_ = Phase::GameOver; return; }
            BeginTurn();
            return;
        }

        MoveJet(input);
        for (int i = 0; i < 2; ++i)
        {
            triggerY_ += speedY_;
            if (triggerY_ >= 256 || speedY_ >= 254) { ++scroll_; triggerY_ &= 255; }
        }
        EnsureCourse();
        // Save each player's own checkpoint as the jet enters the next section.
        for (size_t i = 1; i < sections_.size(); ++i)
            if (scroll_ + JetBottom >= sections_[i].start && sections_[i].number > players_[activePlayer_].segment)
            {
                auto& player = players_[activePlayer_];
                player.difficulty = sections_[i].course.Difficulty();
                player.segment = sections_[i].number;
                player.seed = sections_[i - 1].course.NextSeed();
            }
        UpdateEnemies();
        UpdateMissile(input.fire);
        CheckJetCollisions();
        if (phase_ == Phase::Flying)
        {
            fuel_ = std::max(0, fuel_ - 32);
            if (!fuel_) Crash();
        }
        sound_.engine = phase_ == Phase::Flying;
        sound_.lowFuel = fuel_ < 8192;
        sound_.throttle = float(speedY_ - 64) / 190.0f;
    }
}
