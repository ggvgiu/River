#include "../RiverRaidGame.h"
#include "../CollisionShapes.h"
#include "../GameRules.h"
#include "../SeedValue.h"
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
        static void Moving(RiverRaidGame& game)
        {
            for(auto& object:game.sections_.front().course.Objects()) object.moving=true;
        }
        static void AdvanceEnemies(RiverRaidGame& game) {game.tick_+=2;game.UpdateEnemies();}
        static void Direction(RiverRaidGame& game, int direction)
        {game.sections_.front().course.Objects()[0].direction=direction;}
        static uint8_t EnemyRandom(const RiverRaidGame& game) {return game.enemyRandom_;}
        static void Score(RiverRaidGame& game, int points) { game.AwardPoints(points); }
        static void Missile(RiverRaidGame& game, float x, float y) { game.missile_ = {x,y,true}; }
        static void AdvanceMissile(RiverRaidGame& game) { game.UpdateMissile(false); }
        static void Crash(RiverRaidGame& game) { game.Crash(); }
        static void AtNextLevel(RiverRaidGame& game, int targetSection = 2)
        {
            game.scroll_ = float((targetSection - 1) * LevelLength - 1);
            game.EnsureCourse();
            for (auto& section : game.sections_) section.course.Objects().clear();
        }
        static void AtDifficultyLimit(RiverRaidGame& game)
        {
            game.Reset();
            game.players_[0].difficulty = 48;
            game.players_[0].segment = 48;
            game.BeginTurn();
            game.phase_ = Phase::Flying;
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
        Check(ParseSeed(L"43028") == DefaultSeed, "Decimal seed entry");
        Check(ParseSeed(L" 0xa814 ") == DefaultSeed, "Hex seed entry");
        Check(ParseSeed(L"$A814") == DefaultSeed, "Atari hex entry");
        Check(ParseSeed(L"1") == 1 && ParseSeed(L"0xFFFF") == 65535, "Seed boundaries");
        for (auto invalid : {L"", L" ", L"0", L"0x0000", L"65536", L"0x10000", L"-1", L"1.5", L"0x", L"$", L"A814", L"12junk", L"9999999999999999999999"})
            Check(!ParseSeed(invalid), "Reject invalid seed input");

        RiverRaidGame seeded;
        Check(seeded.BaseSeed() == DefaultSeed, "Original default seed");
        seeded.ToggleDifficulty(0);
        seeded.SelectPlayers();
        Check(seeded.SetBaseSeed(12345), "Accept custom seed");
        Check(seeded.Players()[0].seed == 12345 && seeded.Players()[1].seed == 12345, "Both players use base seed");
        Check(seeded.PlayerCount() == 2 && seeded.Players()[0].expert, "Seed change preserves selected modes");
        const auto customNext = seeded.Sections().front().course.NextSeed();
        Check(customNext == RiverCourse(1,12345).NextSeed(), "Custom seed drives level generation");
        seeded.Reset();
        Check(seeded.BaseSeed() == 12345 && seeded.Sections().front().course.NextSeed() == customNext, "Reset reproduces selected seed");
        Check(!seeded.SetBaseSeed(0) && seeded.BaseSeed() == 12345, "Zero does not mutate game");
        seeded.SetBaseSeed(DefaultSeed);
        Check(seeded.Sections().front().course.NextSeed() == RiverCourse(1,DefaultSeed).NextSeed(), "Restore original course");
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
        RiverRaidGame trailing;
        GameTestAccess::OpenRiver(trailing);
        GameTestAccess::Add(trailing,ObjectKind::Helicopter,76,80);
        GameTestAccess::Moving(trailing);
        auto forwardOnly=trailing;
        GameTestAccess::Add(trailing,ObjectKind::Ship,76,-50);
        GameTestAccess::Add(trailing,ObjectKind::Helicopter,76,-60);
        GameTestAccess::Add(trailing,ObjectKind::Plane,159,-70);
        GameTestAccess::Add(trailing,ObjectKind::Plane,70,-160);
        GameTestAccess::Moving(trailing);
        GameTestAccess::AdvanceEnemies(trailing);
        GameTestAccess::AdvanceEnemies(forwardOnly);
        const auto& trailingObjects=trailing.Sections().front().course.Objects();
        Check(trailingObjects[1].x==77 && trailingObjects[2].x==77,"Passed ships and helicopters keep moving");
        Check(trailingObjects[3].x==0,"Passed enemy planes retain screen wrapping");
        Check(trailingObjects[4].x==70,"Enemies outside retained history stop updating");
        for(int i=0;i<12;++i)
        {
            GameTestAccess::AdvanceEnemies(trailing);
            GameTestAccess::AdvanceEnemies(forwardOnly);
        }
        Check(GameTestAccess::EnemyRandom(trailing)==GameTestAccess::EnemyRandom(forwardOnly),"Passed movement consumes no extra randomness");
        Check(trailingObjects[0].x==forwardOnly.Sections().front().course.Objects()[0].x,
            "Passed movement leaves forward enemy movement unchanged");
        GameTestAccess::OpenRiver(trailing);
        GameTestAccess::Add(trailing,ObjectKind::Helicopter,44,-50);
        GameTestAccess::Moving(trailing);
        // Point toward the bank and verify the existing bounce policy still runs.
        GameTestAccess::Direction(trailing,-1);
        GameTestAccess::AdvanceEnemies(trailing);
        const auto& bouncing=trailing.Sections().front().course.Objects()[0];
        Check(bouncing.direction==1 && bouncing.x==44,"Passed enemies still turn at river banks");
        // The assembly uses randomHi & 1 below stage 3, so a jet must never
        // be generated in either half of displayed level 1, for any seed.
        for (unsigned seed=1;seed<=512;++seed)
            for (int stage:{1,2})
            {
                const RiverCourse course(stage,uint16_t(seed));
                for (const auto& object:course.Objects())
                    Check(object.kind!=ObjectKind::Plane,"No early enemy jets across varied seeds");
            }

        RiverRaidGame audit;
        audit.ToggleInvincibility();
        std::array<PilotInput,2> auditInput{};
        auditInput[0].throttle=1;
        bool foundJet=false;
        for(int frame=0;frame<2400 && !foundJet;++frame)
        {
            audit.Tick(auditInput);
            for(const auto& section:audit.Sections())
                for(const auto& object:section.course.Objects())
                {
                    if(object.kind!=ObjectKind::Plane) continue;
                    Check(section.course.Difficulty()>=3,"Plane belongs to an eligible original stage");
                    const float y=section.start+object.distance-audit.Scroll();
                    const auto shape=ObjectCollisionShape(object.kind,audit.Frame());
                    bool visible=false;
                    for(int row=0;row<shape.Height();++row)
                        if(shape.rows[row] && y+row>=0 && y+row<FieldHeight) visible=true;
                    if(!visible) continue;
                    std::cout<<"AUDIT: first visible enemy jet belongs to stage "<<section.course.Difficulty()
                        <<" (paired level "<<(section.number-1)/2+1<<"), player HUD level "
                        <<audit.Players()[0].Level()<<", scroll "<<audit.Scroll()<<'\n';
                    foundJet=true;
                }
        }
        Check(foundJet,"Default course eventually introduces enemy jets");
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
            auto sprite = ObjectCollisionShape(kind,0);
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
        Check(game.Players()[0].difficulty==2 && game.Players()[0].Level()==1,
            "River and mountains are both level one");
        GameTestAccess::AtNextLevel(game,3);
        game.Tick({});
        Check(game.Players()[0].difficulty==3 && game.Players()[0].Level()==2,
            "Next river starts level two");
        GameTestAccess::AtNextLevel(game,4);
        game.Tick({});
        Check(game.Players()[0].Level()==2,"Second mountains segment remains level two");
        GameTestAccess::Crash(game);
        for(int i=0;i<90;++i) game.Tick({});
        Check(game.Players()[0].Level()==2 && game.Players()[0].segment==4,
            "Restart preserves paired level and segment checkpoint");
        GameTestAccess::AtDifficultyLimit(game);
        Check(game.Players()[0].Level()==24,"Difficulty stage 48 belongs to level 24");
        GameTestAccess::AtNextLevel(game);
        game.Tick({});
        Check(game.Players()[0].difficulty==47 && game.Players()[0].Level()==25,
            "Level counter continues beyond difficulty cap");
        game.Reset();
        Check(game.Players()[0].Level()==1 && game.Players()[1].Level()==1,"Reset starts both players at level one");
        GameTestAccess::OpenRiver(game);
        Check(!game.Cheats().infiniteLives && !game.Cheats().invincible, "Cheats default off");
        game.ToggleInfiniteLives();
        GameTestAccess::Crash(game);
        Check(game.State() == Phase::Crashing && game.Players()[0].lives == 3, "Infinite lives preserves lives but allows crashes");
        for (int i=0;i<90;++i) game.Tick({});
        Check(game.State() == Phase::Ready, "Infinite lives restarts after crash");
        game.ToggleInfiniteLives();

        GameTestAccess::OpenRiver(game);
        game.ToggleInvincibility();
        GameTestAccess::Add(game, ObjectKind::Bridge, 64, 4);
        GameTestAccess::Fuel(game, 32);
        game.Tick({});
        Check(game.State() == Phase::Flying && game.Fuel() == 0 && game.Players()[0].lives == 3,
            "Invincibility prevents bridge and empty-fuel deaths");
        controls = {}; controls[0].horizontal = -1;
        for (int i=0;i<100;++i) game.Tick(controls);
        Check(game.State() == Phase::Flying && game.JetX() < 44, "Invincibility allows flying over banks");
        GameTestAccess::OpenRiver(game);
        GameTestAccess::Fuel(game, 30000);
        GameTestAccess::Add(game, ObjectKind::Fuel, 76, 0);
        game.Tick({});
        Check(game.Refueling(), "Invincibility still permits refueling");
        game.ToggleInvincibility();
        GameTestAccess::Fuel(game, 0);
        GameTestAccess::Crash(game);
        Check(game.State() == Phase::Crashing && game.Players()[0].lives == 2, "Disabling cheats restores deaths");

        game.ToggleInfiniteLives(); game.ToggleInvincibility();
        game.SelectPlayers(); game.SetBaseSeed(1234); game.Reset();
        Check(game.Cheats().infiniteLives && game.Cheats().invincible, "Cheats persist across mode, seed and reset");
        controls = {}; controls[0].fire = true; game.Tick(controls);
        game.ToggleInvincibility();
        GameTestAccess::Crash(game);
        for (int i=0;i<90;++i) game.Tick({});
        Check(game.ActivePlayer() == 1 && game.Players()[0].lives == 3, "Infinite lives retains alternating turns");
        std::cout << "PASS: gameplay, seeds, infinite lives, invincibility, and alternating players\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
