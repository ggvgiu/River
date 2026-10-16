#include "platform/framework.h"
#include "rendering/RiverRaidView.h"
#include "rendering/visuals/AssetDrawing.h"
#include "rendering/visuals/EnvironmentPalette.h"
#include "rendering/ValleyView.h"
#include <gl/GL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string_view>

namespace
{
    constexpr float HudHeight = 40;
    bool monochrome = false;
    void Color(float r, float g, float b)
    {
        if (monochrome) r = g = b = 0.299f * r + 0.587f * g + 0.114f * b;
        glColor3f(r, g, b);
    }
    void Rect(float x, float y, float width, float height)
    {
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + width, y);
        glVertex2f(x + width, y + height); glVertex2f(x, y + height);
        glEnd();
    }

    // Small vector bitmap font; no system font, textures, or external assets needed.
    constexpr std::string_view Characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:-./ ";
    constexpr unsigned char Glyphs[][7] = {
        {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
        {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
        {14,17,17,15,1,1,14},
        {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
        {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
        {7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
        {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
        {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
        {0,4,4,0,4,4,0},{0,0,0,31,0,0,0},{0,0,0,0,0,4,4},
        {1,2,2,4,8,8,16},{0,0,0,0,0,0,0}
    };

    void Text(std::string_view text, float x, float y, float scale = 0.65f)
    {
        for (char ch : text)
        {
            auto index = Characters.find(ch);
            if (index != std::string_view::npos)
                for (int row = 0; row < 7; ++row)
                for (int col = 0; col < 5; ++col)
                    if (Glyphs[index][row] & (16 >> col)) Rect(x + col * scale, y + (6 - row) * scale, scale, scale);
            x += 6 * scale;
        }
    }

    void CenterText(std::string_view text, float y, float scale = 0.65f)
    {
        Text(text, (160 - float(text.size()) * 6 * scale) * 0.5f, y, scale);
    }

    void Explosion(float x, float y, int remaining, float size)
    {
        float radius = size * (1.0f - float(remaining % 24) / 28);
        for (int ray = 0; ray < 12; ++ray)
        {
            float angle = float(ray) * 6.2831853f / 12;
            float distance = radius * (0.6f + float(ray % 3) * 0.2f);
            Color(1, 0.35f + float(ray % 3) * 0.25f, 0.12f);
            Rect(x + std::cos(angle) * distance, y + std::sin(angle) * distance, 1.6f, 2);
        }
    }
}

void RiverRaidView::Draw(const RiverRaid::RiverRaidGame& game, const RenderOptions& options) const
{
    using namespace RiverRaid;
    monochrome = game.Monochrome();
    if (options.valley3D)
    {
        ValleyView().Draw(game, options, assets_);
        DrawOverlay(game);
        return;
    }
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 160, 0, 200, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the top-down game as a plane in the OpenGL scene.
    const auto& palette=Visuals::PaletteForSeed(game.BaseSeed(),options.seedColors);
    Color(palette.topDownWater.r,palette.topDownWater.g,palette.topDownWater.b);
    Rect(0, HudHeight, 160, 160);
    for (int y = 0; y < FieldHeight; ++y)
    {
        const float worldY = game.Scroll() + y;
        const auto row = game.Row(worldY);
        const bool bright = ((game.Players()[game.ActivePlayer()].difficulty & 1) != 0);
        const auto land=bright?palette.topDownGrass:palette.topDownDarkGrass;
        Color(land.r,land.g,land.b);
        Rect(0, HudHeight + y, row.bank, 1);
        Rect(160 - row.bank, HudHeight + y, row.bank, 1);
        if (row.island > 0) Rect(80 - row.island, HudHeight + y, row.island * 2, 1);
        Color(palette.shore.r,palette.shore.g,palette.shore.b);
        Rect(row.bank - 0.45f, HudHeight + y, 0.45f, 1);
        Rect(160 - row.bank, HudHeight + y, 0.45f, 1);
        if (row.island > 0)
        {
            Rect(80 - row.island, HudHeight + y, 0.45f, 1);
            Rect(80 + row.island - 0.45f, HudHeight + y, 0.45f, 1);
        }
        if (row.road)
        {
            Color(0.38f, 0.39f, 0.37f);
            Rect(0, HudHeight + y, row.bank, 1);
            Rect(160 - row.bank, HudHeight + y, row.bank, 1);
            if ((int(worldY) % 64) == 48)
            {
                Color(0.9f, 0.78f, 0.32f);
                for (int x = 0; x < 160; x += 8)
                    if (x < row.bank || x > 160 - row.bank) Rect(float(x), HudHeight + y, 4, 1);
            }
        }
    }

    // Restrict sprites to the river viewport so scrolling never covers the HUD.
    glPushMatrix();
    glTranslatef(0, HudHeight, 0);
    GLdouble bottomClip[] = {0, 1, 0, 0};
    GLdouble topClip[] = {0, -1, 0, 160};
    glClipPlane(GL_CLIP_PLANE0, bottomClip);
    glClipPlane(GL_CLIP_PLANE1, topClip);
    glEnable(GL_CLIP_PLANE0);
    glEnable(GL_CLIP_PLANE1);
    for (const auto& section : game.Sections())
    for (const auto& object : section.course.Objects())
    {
        const float y = section.start + object.distance - game.Scroll();
        if (y < -32 || y > 160) continue;
        const auto& model = assets_.Object(object.kind);
        const auto& frame = model.Frame(game.Frame());
        if (object.destroyed)
        {
            if (object.explosionTicks) Explosion(object.x + frame.width / 2.0f, y + frame.length / 2.0f,
                object.explosionTicks, model.explosionSize);
            continue;
        }
        const bool mirrored = model.mirrorsWithDirection && object.direction < 0;
        Visuals::DrawPixelShadow(frame, object.x + 0.6f, y - 0.7f, mirrored, game.Monochrome(), {0.035f,0.1f,0.21f});
        Visuals::DrawPixels(frame, object.x, y, mirrored, game.Monochrome());
    }
    if (game.Bullet().active)
    {
        Color(1, 0.95f, 0.56f);
        Rect(game.Bullet().x, game.Bullet().y, 1, 3);
    }
    if (game.State() == Phase::Crashing) Explosion(game.JetX() + 4, 12, game.CrashTicks(), 12);
    else if (game.State() != Phase::GameOver)
    {
        const auto& frame = assets_.Player(game.Banking()!=0, game.ActivePlayer()).Frame(game.Frame());
        Visuals::DrawPixelShadow(frame, game.JetX()+0.5f, JetBottom-0.7f, game.Banking()<0,
            game.Monochrome(), {0.025f,0.08f,0.18f});
        Visuals::DrawPixels(frame, game.JetX(), float(JetBottom), game.Banking()<0, game.Monochrome());
    }
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glPopMatrix();

    DrawOverlay(game);
}

void RiverRaidView::DrawOverlay(const RiverRaid::RiverRaidGame& game) const
{
    using namespace RiverRaid;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 160, 0, 200, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Color(0.025f, 0.045f, 0.075f);
    Rect(0, 0, 160, HudHeight);
    Color(0.91f, 0.93f, 0.84f);
    char label[80];
    const auto& pilot = game.Players()[game.ActivePlayer()];
    sprintf_s(label, "P%d %06d  LIVES %d  LEVEL %02d", game.ActivePlayer() + 1, pilot.score, pilot.lives, pilot.Level());
    CenterText(label, 31, 0.72f);
    if (game.PlayerCount() == 2)
    {
        const auto& other = game.Players()[1 - game.ActivePlayer()];
        sprintf_s(label, "P%d %06d  LIVES %d", 2 - game.ActivePlayer(), other.score, other.lives);
        Color(0.53f, 0.65f, 0.73f);
        CenterText(label, 25, 0.52f);
    }
    Color(0.7f, 0.77f, 0.73f);
    Text("E", 25, 16);
    Text("F", 133, 16);
    Color(0.17f, 0.22f, 0.27f);
    Rect(32, 16, 96, 4);
    if (game.Refueling()) Color(0.4f, 1, 0.7f);
    else if (game.Fuel() < 0.125f) Color(0.95f, 0.2f, 0.18f);
    else Color(0.91f, 0.78f, 0.25f);
    Rect(32, 16, 96 * game.Fuel(), 4);
    Color(0.53f, 0.65f, 0.73f);
    CenterText("F1 RESET  F2 SELECT  F3 PAUSE", 7, 0.59f);
    sprintf_s(label, "%d PLAYER%s  %s MISSILES", game.PlayerCount(), game.PlayerCount() == 1 ? "" : "S",
        pilot.expert ? "DIRECT" : "GUIDED");
    CenterText(label, 1, 0.5f);

    std::string_view message;
    if (game.Paused()) message = "PAUSED";
    else if (game.State() == Phase::Ready) message = "READY";
    else if (game.State() == Phase::GameOver) message = "GAME OVER";
    if (!message.empty())
    {
        Color(0.025f, 0.045f, 0.075f);
        Rect(16, 105, 128, 34);
        Color(1, 0.86f, 0.3f);
        CenterText(message, 126, 1.1f);
        Color(0.88f, 0.92f, 0.9f);
        if (game.State() == Phase::Ready && !game.Paused())
            CenterText(game.ActivePlayer() == 0 ? "ARROWS / SPACE TO START" : "WASD / F TO START", 116, 0.7f);
        else CenterText(game.Paused() ? "F3 TO CONTINUE" : "F1 TO PLAY AGAIN", 116, 0.7f);
        CenterText("RIVER RAID - CAROL SHAW 1982", 108, 0.56f);
    }
}
