#pragma once
#include <cmath>
#include <cstdint>

namespace RiverRaid::Visuals
{
    // Stateless gradient Perlin noise. Never advances the gameplay generator.
    inline float Perlin(float x,float y,uint32_t seed)
    {
        const int ix=int(std::floor(x)),iy=int(std::floor(y));
        const float fx=x-ix,fy=y-iy;
        auto gradient=[&](int gx,int gy,float dx,float dy) {
            uint32_t h=uint32_t(gx)*0x9e3779b9u ^ uint32_t(gy)*0x85ebca6bu ^ seed;
            h^=h>>16;h*=0x7feb352du;h^=h>>15;h*=0x846ca68bu;h^=h>>16;
            constexpr float diagonal=0.70710678f;
            switch(h&7)
            {
            case 0:return dx;case 1:return -dx;case 2:return dy;case 3:return -dy;
            case 4:return (dx+dy)*diagonal;case 5:return (dx-dy)*diagonal;
            case 6:return (-dx+dy)*diagonal;default:return (-dx-dy)*diagonal;
            }
        };
        auto fade=[](float t){return t*t*t*(t*(t*6-15)+10);};
        auto mix=[](float a,float b,float t){return a+(b-a)*t;};
        return mix(mix(gradient(ix,iy,fx,fy),gradient(ix+1,iy,fx-1,fy),fade(fx)),
            mix(gradient(ix,iy+1,fx,fy-1),gradient(ix+1,iy+1,fx-1,fy-1),fade(fx)),fade(fy));
    }
}
