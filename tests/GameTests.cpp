#include "../RiverRaidGame.h"
#include "../SpriteAssets.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace RiverRaid
{
    // Controlled fixtures let tests isolate collisions without Windows or OpenGL.
    struct GameTestAccess
    {
        static void OpenRiver(RiverRaidGame& game)
        {
            game.Reset();
            game.phase_ = Phase::Flying;
            game.scroll_ = 100;
            for (auto& section : game.sections_) section.course.Objects().clear();
        }
        static void Add(RiverRaidGame& game, ObjectKind kind, float x, float screenY)
        {
            game.sections_.front().course.Objects().push_back({kind, x, game.scroll_ + screenY});
        }
        static void Fuel(RiverRaidGame& game, int amount) { game.fuel_ = amount; }
        static void Score(RiverRaidGame& game, int points) { game.AwardPoints(points); }
        static void Missile(RiverRaidGame& game, float x, float y) { game.missile_ = {x,y,true}; }
        static void AdvanceMissile(RiverRaidGame& game) { game.UpdateMissile(false); }
        static void Crash(RiverRaidGame& game) { game.Crash(); }
        static void AtNextLevel(RiverRaidGame& game)
        {
            game.scroll_ = 1023;
            game.EnsureCourse();
            for (auto& section : game.sections_) section.course.Objects().clear();
        }
    };
}

void Check(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main()
{
    using namespace RiverRaid;
    try
    {
        CourseRandom rng;
        rng.Next(); Check(rng.Value() == 0x5029, "Original LFSR first step");
        rng.Next(); Check(rng.Value() == 0xA053, "Original LFSR second step");
        RiverCourse a(2,0xA814), b(2,0xA814);
        Check(a.Objects().size() == 32, "32 zones per river level");
        Check(a.Objects().back().kind == ObjectKind::Bridge, "Last zone is bridge");
        Check(a.Objects().front().kind == ObjectKind::House, "First zone is house");
        Check(a.NextSeed() == b.NextSeed(), "Repeatable checkpoints");
        for (int y = 0; y < LevelLength; ++y)
        {
            Check(a.Row(float(y)).bank == b.Row(float(y)).bank, "Repeatable banks");
            Check(a.Row(float(y)).island == b.Row(float(y)).island, "Repeatable islands");
        }
        for (int level = 1; level <= 48; ++level)
        {
            RiverCourse course(level, 0xA814);
            for (int y = 0; y < LevelLength; ++y)
            {
                const auto row = course.Row(float(y));
                Check(row.bank >= 16 && row.bank <= 72, "Bank bounds at all difficulties");
                Check(80 - row.bank - row.island >= 8, "River channels wide enough for plane");
            }
            if (level < 3)
                for (const auto& object : course.Objects()) Check(object.kind != ObjectKind::Plane, "No enemy jets before level three");
        }

        std::array<PilotInput,2> controls{};
        RiverRaidGame game;
        Check(game.State() == Phase::Ready && game.Players()[0].lives == 3, "Three starting lives");
        game.Tick(controls);
        Check(game.Scroll() == -48, "Wait for joystick before launch");
        controls[0].fire = true;
        game.Tick(controls);
        Check(game.State() == Phase::Flying && game.Bullet().active, "Fire starts game and missile");
        game.TogglePause();
        const float scroll = game.Scroll(), fuel = game.Fuel();
        for (int i=0;i<60;++i) game.Tick(controls);
        Check(game.Scroll() == scroll && game.Fuel() == fuel, "Pause freezes gameplay");

        GameTestAccess::OpenRiver(game);
        controls = {};
        for (int i=0;i<60;++i) game.Tick(controls);
        Check(game.Scroll() == 160, "Cruise speed is 60 scanlines/second");
        Check(std::abs(game.Fuel() - float(65535-60*32)/65535) < 0.0001f, "Original fuel drain");
        controls[0].throttle = 1;
        for (int i=0;i<70;++i) game.Tick(controls);
        float fastStart = game.Scroll();
        for (int i=0;i<60;++i) game.Tick(controls);
        Check(game.Scroll() - fastStart == 120, "Maximum speed is two scanlines/frame");

        GameTestAccess::OpenRiver(game);
        GameTestAccess::Fuel(game, 30000);
        GameTestAccess::Add(game, ObjectKind::Fuel, 76, 6);
        controls = {};
        game.Tick(controls);
        Check(game.Refueling() && game.Fuel() > float(30000)/65535, "Overlap refuels without crashing");

        for (ObjectKind kind : {ObjectKind::Ship, ObjectKind::Helicopter, ObjectKind::Plane, ObjectKind::Fuel, ObjectKind::Bridge})
        {
            GameTestAccess::OpenRiver(game);
            GameTestAccess::Add(game, kind, 76, 50);
            // Aim at a known occupied mask pixel and sweep through it.
            auto sprite = ObjectSprite(kind,0);
            int targetX=0,targetY=0;
            for (int y=0;y<sprite.Height();++y)
                for(int x=0;x<sprite.Width();++x)
                    if(sprite.Solid(x,y)) { targetX=x;targetY=y; }
            if (!game.Players()[0].expert) game.ToggleDifficulty(0);
            GameTestAccess::Missile(game, float(76+targetX), float(50+targetY-4));
            GameTestAccess::AdvanceMissile(game);
            Check(game.Players()[0].score == ObjectScore(kind), "Original object scores");
            Check(!game.Bullet().active, "Single missile consumed on hit");
            Check(game.Sections().front().course.Objects()[0].destroyed, "Hit removes object");
        }

        GameTestAccess::OpenRiver(game);
        GameTestAccess::Fuel(game,32);
        game.Tick({});
        Check(game.State() == Phase::Crashing && game.Players()[0].lives == 2, "Fuel exhaustion loses life");

        GameTestAccess::OpenRiver(game);
        GameTestAccess::Add(game,ObjectKind::Bridge,64,4);
        game.Tick({});
        Check(game.State()==Phase::Crashing,"Intact bridge blocks player");

        GameTestAccess::OpenRiver(game);
        if (game.Players()[0].expert) game.ToggleDifficulty(0);
        GameTestAccess::Missile(game,60,50);
        GameTestAccess::AdvanceMissile(game);
        Check(game.Bullet().x==game.JetX()+5,"Beginner missile follows plane");
        game.ToggleDifficulty(0);
        GameTestAccess::Missile(game,60,50);
        GameTestAccess::AdvanceMissile(game);
        Check(game.Bullet().x==60,"Expert missile flies straight");

        GameTestAccess::OpenRiver(game);
        controls = {};
        controls[0].horizontal = -1;
        for(int i=0;i<80 && game.State()==Phase::Flying;++i) game.Tick(controls);
        Check(game.State() == Phase::Crashing, "Bank collision uses silhouette");

        GameTestAccess::OpenRiver(game);
        GameTestAccess::Score(game,10000);
        Check(game.Players()[0].lives == 4, "Bonus life at 10000 points");
        for(int i=0;i<12;++i) GameTestAccess::Score(game,10000);
        Check(game.Players()[0].lives == 9, "Nine-life cap");

        game.SelectPlayers();
        Check(game.PlayerCount()==2, "Select enables two players");
        controls = {}; controls[0].fire=true;
        game.Tick(controls);
        GameTestAccess::Crash(game);
        for(int i=0;i<90;++i) game.Tick({});
        Check(game.ActivePlayer()==1 && game.State()==Phase::Ready, "Alternate players after crash");
        Check(game.Players()[0].lives==2 && game.Players()[1].lives==3, "Independent lives");
        controls = {}; controls[1].fire=true;
        game.Tick(controls);
        Check(game.State()==Phase::Flying && game.Bullet().active, "Second joystick controls second turn");

        game.SelectPlayers();
        for(int life=0;life<3;++life)
        {
            controls={};controls[0].fire=true;
            game.Tick(controls);
            GameTestAccess::Crash(game);
            for(int i=0;i<90;++i) game.Tick({});
        }
        Check(game.State()==Phase::GameOver,"Three deaths end one-player game");
        game.Reset();
        Check(game.State()==Phase::Ready && game.Players()[0].lives==3,"Reset restores playable game");

        GameTestAccess::OpenRiver(game);
        GameTestAccess::AtNextLevel(game);
        game.Tick({});
        Check(game.Players()[0].level==2, "Checkpoint advances at next level");
        std::cout << "PASS: generation, all 48 difficulties, timing, fuel, collisions, scoring, missiles, lives, checkpoints, alternating players\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
