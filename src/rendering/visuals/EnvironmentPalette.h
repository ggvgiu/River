#pragma once
#include "rendering/visuals/AssetModel.h"
#include "game/RiverCourse.h"
#include <array>
#include <algorithm>
#include <cmath>

namespace RiverRaid::Visuals
{
    struct EnvironmentPalette
    {
        const wchar_t* name;
        Color water,waterRipple,grass,rock,wall,sky;
        Color topDownWater,topDownGrass,topDownDarkGrass,shore;
    };
    inline constexpr std::array<EnvironmentPalette,16> EnvironmentPalettes{{
        {L"Classic",{.06f,.23f,.39f},{.035f,.09f,.12f},{.24f,.43f,.16f},{.37f,.40f,.31f},{.19f,.32f,.12f},{.32f,.43f,.48f},
            {.07f,.19f,.42f},{.27f,.48f,.11f},{.19f,.38f,.11f},{.49f,.58f,.23f}},
        {L"Lava",{.86f,.20f,.025f},{.12f,.22f,.035f},{.30f,.15f,.12f},{.43f,.26f,.22f},{.19f,.09f,.08f},{.35f,.20f,.18f},
            {.86f,.20f,.025f},{.39f,.20f,.13f},{.27f,.13f,.11f},{.68f,.34f,.12f}},
        {L"Amethyst",{.31f,.10f,.53f},{.14f,.09f,.18f},{.43f,.21f,.18f},{.61f,.38f,.31f},{.29f,.13f,.12f},{.36f,.29f,.46f},
            {.31f,.10f,.53f},{.49f,.27f,.21f},{.36f,.18f,.17f},{.68f,.44f,.34f}},
        {L"Desert",{.055f,.39f,.40f},{.035f,.12f,.12f},{.55f,.37f,.20f},{.73f,.57f,.38f},{.36f,.23f,.13f},{.54f,.48f,.38f},
            {.055f,.39f,.40f},{.62f,.43f,.24f},{.46f,.30f,.17f},{.79f,.65f,.41f}},
        {L"Glacier",{.12f,.37f,.59f},{.10f,.15f,.16f},{.43f,.58f,.58f},{.81f,.87f,.88f},{.29f,.41f,.46f},{.55f,.65f,.73f},
            {.12f,.37f,.59f},{.63f,.75f,.72f},{.43f,.59f,.60f},{.85f,.91f,.87f}},
        {L"Marsh",{.21f,.29f,.065f},{.09f,.12f,.04f},{.31f,.34f,.14f},{.46f,.38f,.25f},{.20f,.23f,.10f},{.36f,.39f,.27f},
            {.21f,.29f,.065f},{.36f,.43f,.16f},{.26f,.32f,.13f},{.56f,.57f,.26f}},
        {L"Obsidian",{.10f,.08f,.18f},{.10f,.07f,.16f},{.21f,.24f,.28f},{.42f,.43f,.49f},{.11f,.13f,.17f},{.25f,.26f,.35f},
            {.10f,.08f,.18f},{.28f,.31f,.35f},{.18f,.20f,.25f},{.52f,.46f,.62f}},
        {L"Copper",{.12f,.48f,.36f},{.05f,.14f,.12f},{.54f,.25f,.14f},{.75f,.45f,.26f},{.34f,.15f,.09f},{.47f,.36f,.28f},
            {.12f,.48f,.36f},{.62f,.30f,.17f},{.44f,.20f,.12f},{.83f,.58f,.33f}},
        {L"Emerald",{.025f,.43f,.25f},{.05f,.18f,.11f},{.18f,.35f,.30f},{.38f,.52f,.45f},{.10f,.23f,.20f},{.27f,.45f,.39f},
            {.025f,.43f,.25f},{.24f,.43f,.34f},{.14f,.29f,.24f},{.50f,.68f,.39f}},
        {L"Rose",{.57f,.16f,.36f},{.17f,.09f,.14f},{.43f,.29f,.40f},{.72f,.53f,.62f},{.29f,.18f,.27f},{.48f,.35f,.44f},
            {.57f,.16f,.36f},{.51f,.34f,.44f},{.35f,.23f,.33f},{.83f,.60f,.66f}},
        {L"Midnight",{.035f,.075f,.25f},{.04f,.08f,.16f},{.14f,.20f,.29f},{.29f,.34f,.49f},{.08f,.11f,.18f},{.11f,.15f,.27f},
            {.035f,.075f,.25f},{.20f,.27f,.36f},{.12f,.17f,.25f},{.40f,.47f,.68f}},
        {L"Alien",{.48f,.62f,.035f},{.14f,.18f,.04f},{.33f,.13f,.41f},{.57f,.32f,.64f},{.21f,.08f,.29f},{.38f,.28f,.43f},
            {.48f,.62f,.035f},{.40f,.20f,.48f},{.28f,.11f,.35f},{.66f,.42f,.73f}},
        {L"Autumn",{.08f,.26f,.36f},{.05f,.11f,.13f},{.56f,.31f,.10f},{.69f,.48f,.26f},{.36f,.20f,.08f},{.47f,.42f,.33f},
            {.08f,.26f,.36f},{.66f,.38f,.13f},{.46f,.25f,.09f},{.85f,.62f,.24f}},
        {L"Chalk",{.12f,.53f,.62f},{.07f,.15f,.16f},{.67f,.65f,.54f},{.87f,.85f,.76f},{.45f,.44f,.36f},{.58f,.63f,.62f},
            {.12f,.53f,.62f},{.74f,.72f,.59f},{.57f,.56f,.45f},{.91f,.88f,.70f}},
        {L"Crimson",{.58f,.035f,.09f},{.19f,.045f,.07f},{.31f,.29f,.25f},{.53f,.44f,.35f},{.20f,.18f,.15f},{.38f,.25f,.27f},
            {.58f,.035f,.09f},{.40f,.36f,.29f},{.26f,.24f,.20f},{.65f,.53f,.40f}},
        {L"Golden",{.73f,.49f,.045f},{.17f,.17f,.05f},{.21f,.29f,.37f},{.42f,.48f,.55f},{.12f,.18f,.25f},{.43f,.42f,.34f},
            {.73f,.49f,.045f},{.28f,.36f,.44f},{.17f,.24f,.32f},{.59f,.64f,.65f}}
    }};

    inline constexpr size_t PaletteVariants=64;
    inline uint32_t PaletteHash(uint16_t seed)
    {
        uint32_t hash=uint32_t(seed^DefaultSeed)*0x9e3779b9u;
        hash^=hash>>16;
        return hash;
    }
    inline size_t PaletteThemeIndex(uint16_t seed,bool enabled)
    {
        return !enabled || seed==DefaultSeed?0:1+PaletteHash(seed)%(EnvironmentPalettes.size()-1);
    }
    inline Color ShiftColor(Color color,float hueShift,float brightness)
    {
        const float maximum=std::max({color.r,color.g,color.b}),minimum=std::min({color.r,color.g,color.b});
        const float chroma=maximum-minimum;
        float hue=0;
        if(chroma>0)
        {
            if(maximum==color.r)hue=(color.g-color.b)/chroma;
            else if(maximum==color.g)hue=2+(color.b-color.r)/chroma;
            else hue=4+(color.r-color.g)/chroma;
        }
        hue=hue/6+hueShift;hue-=std::floor(hue);
        const float value=std::clamp(maximum*brightness,0.0f,0.95f);
        const float saturation=maximum>0?chroma/maximum:0;
        const float c=value*saturation,x=c*(1-std::abs(std::fmod(hue*6,2.0f)-1)),m=value-c;
        switch(int(hue*6))
        {
        case 0:return {c+m,x+m,m};case 1:return {x+m,c+m,m};case 2:return {m,c+m,x+m};
        case 3:return {m,x+m,c+m};case 4:return {x+m,m,c+m};default:return {c+m,m,x+m};
        }
    }
    inline const EnvironmentPalette& PaletteForSeed(uint16_t seed,bool enabled)
    {
        const size_t theme=PaletteThemeIndex(seed,enabled);
        if(theme==0)return EnvironmentPalettes[0];
        // Build the small visual library once, never per frame. Stable addresses
        // also let the terrain cache recognize changes without hashing colors.
        static const auto variations=[] {
            std::array<std::array<EnvironmentPalette,PaletteVariants>,EnvironmentPalettes.size()> result{};
            for(size_t family=1;family<EnvironmentPalettes.size();++family)
            for(size_t variant=0;variant<PaletteVariants;++variant)
            {
                auto palette=EnvironmentPalettes[family];
                if(variant)
                {
                    const float waterHue=(float(variant&3)-1.5f)*0.018f;
                    const float landHue=(float((variant>>4)&3)-1.5f)*0.022f;
                    const float brightness=0.88f+float((variant>>2)&3)*0.08f;
                    palette.water=ShiftColor(palette.water,waterHue,brightness);
                    palette.topDownWater=ShiftColor(palette.topDownWater,waterHue,brightness);
                    for(auto* color:{&palette.grass,&palette.rock,&palette.wall,&palette.topDownGrass,
                        &palette.topDownDarkGrass,&palette.shore})*color=ShiftColor(*color,landHue,brightness);
                    palette.sky=ShiftColor(palette.sky,(waterHue+landHue)*0.5f,brightness);
                }
                result[family][variant]=palette;
            }
            return result;
        }();
        return variations[theme][(PaletteHash(seed)>>8)%PaletteVariants];
    }
}
