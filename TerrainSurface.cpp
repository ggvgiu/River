#include "framework.h"
#include "TerrainSurface.h"
#include "RenderOptions.h"
#include "visuals/AssetDrawing.h"
#include "visuals/VisualNoise.h"
#include "visuals/EnvironmentPalette.h"
#include <gl/GL.h>
#include <algorithm>
#include <cmath>
#include <array>
#include <vector>
#include <cstddef>
#include <bit>

namespace
{
    using namespace RiverRaid;
    using namespace RiverRaid::Visuals;
    constexpr int CrossSteps=16;
    constexpr int RowStep=2;

    RiverRow SampleRow(const RiverRaidGame& game,float worldY)
    {
        const float first=std::floor(worldY);
        const float fraction=worldY-first;
        const auto a=game.Row(first);
        if(fraction==0)return a;
        const auto b=game.Row(first+1);
        return {a.bank+(b.bank-a.bank)*fraction,a.island+(b.island-a.island)*fraction,a.road};
    }
    float BankHeight(float distance) {return 10+std::min(24.0f,std::max(0.0f,distance)*0.4f);}
    float LandHeight(const RiverRaidGame& game,float x,float worldY,float distance)
    {
        const float inland=std::clamp(distance/30,0.0f,1.0f);
        const float relief=18*Perlin(x/55+0.37f,worldY/65+0.71f,game.BaseSeed())
            +6*Perlin(x/19+0.23f,worldY/25+0.41f,game.BaseSeed()^0x4657u);
        return BankHeight(distance)+inland*relief;
    }
    Color LandColor(float height,bool road,const EnvironmentPalette& palette)
    {
        if(road)return {0.40f,0.40f,0.36f};
        const float stone=std::clamp((height-23)/11,0.0f,1.0f);
        if(&palette==&EnvironmentPalettes[0])
            return {0.24f+stone*0.13f,0.43f-stone*0.03f,0.16f+stone*0.15f};
        return {palette.grass.r+stone*(palette.rock.r-palette.grass.r),
            palette.grass.g+stone*(palette.rock.g-palette.grass.g),
            palette.grass.b+stone*(palette.rock.b-palette.grass.b)};
    }
    Color TerrainColor(const RiverRaidGame& game,float x,float worldY,float height,float distance,bool road,const EnvironmentPalette& palette,float side=0)
    {
        auto color=LandColor(height,road,palette);
        // Broad directional shading makes the relief legible without textures.
        if(side==0)side=x<80?1.0f:-1.0f;
        const float across=LandHeight(game,x-1,worldY,distance+side)-LandHeight(game,x+1,worldY,distance-side);
        const float along=LandHeight(game,x,worldY-1,distance)-LandHeight(game,x,worldY+1,distance);
        const float shade=std::clamp(0.9f+across*0.12f+along*0.18f,0.62f,1.15f);
        return {color.r*shade,color.g*shade,color.b*shade};
    }

    float IslandHeight(const RiverRaidGame& game,float x,float worldY,float halfWidth)
    {
        return 2+LandHeight(game,x,worldY,std::max(0.0f,halfWidth-std::abs(x-80)));
    }
    struct Point {float x,y,z;Color color;};
    using Mesh=std::vector<Point>;
    struct DrawRange {GLint first;GLsizei count;std::array<float,3> low,high;};
    using Planes=std::array<std::array<float,4>,6>;
    Planes CameraPlanes()
    {
        float model[16],projection[16],clip[16]{};
        glGetFloatv(GL_MODELVIEW_MATRIX,model);glGetFloatv(GL_PROJECTION_MATRIX,projection);
        for(int c=0;c<4;++c)for(int r=0;r<4;++r)for(int k=0;k<4;++k)
            clip[c*4+r]+=projection[k*4+r]*model[c*4+k];
        Planes planes{};
        for(int axis=0;axis<3;++axis)for(int sign=0;sign<2;++sign)for(int c=0;c<4;++c)
            planes[axis*2+sign][c]=clip[c*4+3]+(sign?-1.0f:1.0f)*clip[c*4+axis];
        return planes;
    }
    std::vector<DrawRange> MeshRanges(const Mesh& mesh)
    {
        std::vector<DrawRange> ranges;
        constexpr size_t chunk=96; // Whole triangles, grouped for cheap rejection.
        for(size_t first=0;first<mesh.size();first+=chunk)
        {
            const auto& p=mesh[first];
            DrawRange range{GLint(first),GLsizei(std::min(chunk,mesh.size()-first)),{p.x,p.y,p.z},{p.x,p.y,p.z}};
            for(size_t i=first;i<first+range.count;++i)
            {
                const std::array<float,3> position{mesh[i].x,mesh[i].y,mesh[i].z};
                for(int axis=0;axis<3;++axis)
                {
                    range.low[axis]=std::min(range.low[axis],position[axis]);
                    range.high[axis]=std::max(range.high[axis],position[axis]);
                }
            }
            ranges.push_back(range);
        }
        return ranges;
    }
    bool Visible(const DrawRange& range,const Planes& planes)
    {
        for(const auto& plane:planes)
        {
            float distance=plane[3];
            for(int axis=0;axis<3;++axis)
                distance+=plane[axis]*(plane[axis]>=0?range.high[axis]:range.low[axis]);
            if(distance < -0.01f)return false; // Conservative edge tolerance.
        }
        return true;
    }
    void Quad(Mesh& mesh,Point a,Point b,Point c,Point d,bool mono)
    {
        for(auto p:{a,b,c,a,c,d})
        {
            if(mono)p.color.r=p.color.g=p.color.b=0.299f*p.color.r+0.587f*p.color.g+0.114f*p.color.b;
            mesh.push_back(p);
        }
    }
    void Submit(const Mesh& mesh)
    {
        if(mesh.empty())return;
        glVertexPointer(3,GL_FLOAT,sizeof(Point),&mesh.front().x);
        glColorPointer(3,GL_FLOAT,sizeof(Point),&mesh.front().color);
        glDrawArrays(GL_TRIANGLES,0,GLsizei(mesh.size()));
    }
    void SubmitVisible(const Mesh& mesh,const std::vector<DrawRange>& ranges,const Planes& planes)
    {
        if(mesh.empty())return;
        glVertexPointer(3,GL_FLOAT,sizeof(Point),&mesh.front().x);
        glColorPointer(3,GL_FLOAT,sizeof(Point),&mesh.front().color);
        GLint first=0;GLsizei count=0;
        for(const auto& range:ranges)
        {
            if(Visible(range,planes))
            {
                if(count==0)first=range.first;
                count+=range.count;
            }
            else if(count){glDrawArrays(GL_TRIANGLES,first,count);count=0;}
        }
        if(count)glDrawArrays(GL_TRIANGLES,first,count);
    }
    void BankStrip(Mesh& mesh,const RiverRaidGame& game,float worldA,float worldB,bool right,bool mono,const EnvironmentPalette& palette)
    {
        const auto a=SampleRow(game,worldA),b=SampleRow(game,worldB);
        const float za=-worldA,zb=-worldB;
        auto point=[&](RiverRow row,float t,float z,float worldY) {
            const float left=-80+(row.bank+80)*t;
            const float x=right?160-left:left;
            const float height=LandHeight(game,x,worldY,row.bank-left);
            return Point{x,height,z,TerrainColor(game,x,worldY,height,row.bank-left,row.road,palette)};
        };
        std::array<Point,CrossSteps+1> first,second;
        for(int step=0;step<=CrossSteps;++step)
        {
            const float t=float(step)/CrossSteps;
            first[step]=point(a,t,za,worldA);second[step]=point(b,t,zb,worldB);
        }
        for(int step=0;step<CrossSteps;++step)
            Quad(mesh,first[step],first[step+1],second[step+1],second[step],mono);
        // Extend the plateau without stretching the detailed shoreline mesh.
        const float inner=right?240.0f:-80.0f,outer=right?1184.0f:-1024.0f;
        auto outerPoint=[&](float x,float worldY,float z) {
            const float height=LandHeight(game,x,worldY,80);
            return Point{x,height,z,TerrainColor(game,x,worldY,height,80,false,palette)};
        };
        const int steps=int(std::abs(outer-inner)/16);
        Point previousA=outerPoint(inner,worldA,za),previousB=outerPoint(inner,worldB,zb);
        for(int step=0;step<steps;++step)
        {
            const float x1=inner+(outer-inner)*(step+1)/steps;
            const auto nextA=outerPoint(x1,worldA,za),nextB=outerPoint(x1,worldB,zb);
            Quad(mesh,previousA,nextA,nextB,previousB,mono);
            previousA=nextA;previousB=nextB;
        }
        // Only exposed shoreline walls: no internal coplanar box faces.
        const float xa=right?160-a.bank:a.bank,xb=right?160-b.bank:b.bank;
        const Color wall=palette.wall;
        Quad(mesh,{xa,0,za,wall},{xb,0,zb,wall},{xb,10,zb,wall},{xa,10,za,wall},mono);
    }

    struct CachedRow
    {
        int world=0,waterTick=-1;
        uint16_t seed=0;
        bool mono=false,valid=false;
        const EnvironmentPalette* palette=nullptr;
        RiverRow a{},b{};
        Mesh land,water;
        std::vector<DrawRange> landRanges;
    };
    bool SameRow(RiverRow a,RiverRow b) {return a.bank==b.bank && a.island==b.island && a.road==b.road;}
}

namespace TerrainSurface
{
    static float HeightOnRow(const RiverRaidGame& game,float x,float worldY,RiverRow row);
    float FlightAltitude(const RiverRaidGame& game,const AssetFrame& frame,
        float x,float worldY,float cruiseAltitude,bool mirrored)
    {
        float altitude=cruiseAltitude;
        // A spatial envelope anticipates cliffs, producing a continuous descent
        // into the river and climb out again without changing enemy simulation.
        constexpr float approach=20;
        for(const auto& box:frame.boxes)
        {
            const float left=x+(mirrored?frame.width-box.x-box.width:box.x);
            for(float dy=0;dy<=box.length;dy+=1)
            {
                const float sampleY=worldY+box.forward+dy;
                const auto row=SampleRow(game,sampleY);
                for(float sampleX=std::floor(left-approach);sampleX<=std::ceil(left+box.width+approach);sampleX+=1)
                {
                    const float dx=sampleX-left;
                    const float distance=std::max({-dx,dx-box.width,0.0f});
                    const float t=std::clamp(1-distance/approach,0.0f,1.0f);
                    const float blend=t*t*(3-2*t);
                    const float terrain=HeightOnRow(game,sampleX,sampleY,row);
                    altitude=std::max(altitude,(terrain+3-box.bottom)*blend);
                }
            }
        }
        return altitude;
    }

    static float HeightOnRow(const RiverRaidGame& game,float x,float worldY,RiverRow row)
    {
        const bool island=row.island>0 && x>=80-row.island && x<=80+row.island;
        if(!island && x>=row.bank && x<=160-row.bank)return 0;
        // Enemy fly-bys and foundations revisit the same world samples many
        // times. Validate against river geometry as well as the visual seed:
        // a player switch can change a course at the same world coordinates.
        struct Sample
        {
            float x=0,world=0,bank=0,island=0,height=0;
            uint16_t seed=0;
            bool valid=false;
        };
        static thread_local std::array<Sample,16384> samples;
        uint32_t hash=std::bit_cast<uint32_t>(x)*0x9e3779b9u ^ std::bit_cast<uint32_t>(worldY);
        hash^=hash>>16;hash*=0x7feb352du;hash^=hash>>15;
        auto& sample=samples[hash%samples.size()];
        if(sample.valid && sample.x==x && sample.world==worldY && sample.bank==row.bank &&
            sample.island==row.island && sample.seed==game.BaseSeed())return sample.height;
        const float height=island?IslandHeight(game,x,worldY,row.island):
            LandHeight(game,x,worldY,row.bank-std::min(x,160-x));
        sample={x,worldY,row.bank,row.island,height,game.BaseSeed(),true};
        return height;
    }

    float Height(const RiverRaidGame& game,float x,float worldY)
    {
        return HeightOnRow(game,x,worldY,SampleRow(game,worldY));
    }

    void Draw(const RiverRaidGame& game,bool monochrome,bool seedColors)
    {
        const auto& palette=PaletteForSeed(game.BaseSeed(),seedColors);
        const float start=std::floor((game.Scroll()-ValleyHistoryLines)/RowStep)*RowStep;
        const float end=game.Scroll()+FieldHeight;
        glShadeModel(GL_SMOOTH);
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_COLOR_ARRAY);
        glPushMatrix();glTranslatef(0,0,game.Scroll());
        const auto planes=CameraPlanes();
        // CPU-only ring cache: safe across GL contexts, bounded to retained rows.
        static thread_local std::array<CachedRow,256> cache;
        // World-anchored color ripples keep hulls and shadows on the same plane.
        // Frame time freezes naturally while paused. No gameplay RNG is used.
        const int waterTick=(game.Frame()%360000)/6;
        const float time=float(waterTick)/10;
        auto waterPoint=[&](float x,float worldY) {
            const float ripple=Perlin(x/12+time*0.12f,worldY/18-time*0.18f,game.BaseSeed()^0x57415445u)
                +0.4f*Perlin(x/5-time*0.08f,worldY/7+time*0.1f,game.BaseSeed());
            return Point{x,0,-worldY,
                {palette.water.r+ripple*palette.waterRipple.r,
                    palette.water.g+ripple*palette.waterRipple.g,palette.water.b+ripple*palette.waterRipple.b}};
        };
        for(float worldY=start;worldY<end;worldY+=RowStep)
        {
            const float next=worldY+RowStep;
            const auto a=SampleRow(game,worldY),b=SampleRow(game,next);
            auto& row=cache[uint32_t(int(worldY)/RowStep)%cache.size()];
            const bool rebuild=!row.valid || row.world!=int(worldY) || row.seed!=game.BaseSeed() ||
                row.mono!=monochrome || row.palette!=&palette || !SameRow(row.a,a) || !SameRow(row.b,b);
            if(rebuild)
            {
                row.valid=true;row.world=int(worldY);row.seed=game.BaseSeed();row.mono=monochrome;
                row.palette=&palette;
                row.a=a;row.b=b;row.land.clear();row.waterTick=-1;
                BankStrip(row.land,game,worldY,next,false,monochrome,palette);
                BankStrip(row.land,game,worldY,next,true,monochrome,palette);
                const float ya=worldY,yb=next;
                if(a.island>0 || b.island>0)
                {
                    // Shared vertices and the same seeded relief as the banks.
                    // At both shorelines the relief fades to the original height.
                    constexpr int islandSteps=16;
                    std::array<Point,islandSteps+1> first,second;
                    auto islandPoint=[&](RiverRow river,float world,int step) {
                        const float x=80-river.island+2*river.island*float(step)/islandSteps;
                        const float distance=std::max(0.0f,river.island-std::abs(x-80));
                        const float height=IslandHeight(game,x,world,river.island);
                        return Point{x,height,-world,TerrainColor(game,x,world,height,distance,river.road,palette,x<80?-1.0f:1.0f)};
                    };
                    for(int step=0;step<=islandSteps;++step)
                    {
                        first[step]=islandPoint(a,worldY,step);second[step]=islandPoint(b,next,step);
                    }
                    for(int step=0;step<islandSteps;++step)
                        Quad(row.land,first[step],first[step+1],second[step+1],second[step],monochrome);
                    const Color side{palette.wall.r+0.01f,palette.wall.g+0.02f,palette.wall.b};
                    Quad(row.land,{80-a.island,0,-ya,side},{80-a.island,12,-ya,side},
                        {80-b.island,12,-yb,side},{80-b.island,0,-yb,side},monochrome);
                    Quad(row.land,{80+a.island,0,-ya,side},{80+b.island,0,-yb,side},
                        {80+b.island,12,-yb,side},{80+a.island,12,-ya,side},monochrome);
                }
            }
            if(rebuild)row.landRanges=MeshRanges(row.land);
            if(row.waterTick!=waterTick)
            {
                row.water.clear();row.waterTick=waterTick;
                std::array<Point,41> first,second;
                for(int i=0;i<=40;++i){first[i]=waterPoint(float(i*4),worldY);second[i]=waterPoint(float(i*4),next);}
                for(int i=0;i<40;++i) Quad(row.water,first[i],first[i+1],second[i+1],second[i],monochrome);
            }
            Submit(row.water);SubmitVisible(row.land,row.landRanges,planes);
        }
        glPopMatrix();glPopClientAttrib();
        glShadeModel(GL_FLAT);
    }

    AssetFrame GroundFrame(const RiverRaidGame& game,const AssetFrame& frame,float x,float worldY,bool mirrored)
    {
        AssetFrame result=frame;
        for(size_t index=0;index<frame.groundAnchors.size();++index)
        {
            const auto& anchor=frame.groundAnchors[index];
            const float localX=mirrored?frame.width-anchor.x-anchor.width:anchor.x;
            float base=0;
            for(float dy=0;dy<=anchor.length;dy+=0.5f)
                for(float dx=0;dx<=anchor.width;dx+=0.5f)
                    base=std::max(base,Height(game,x+localX+dx,worldY+anchor.forward+dy));
            for(auto& part:result.boxes)
                if(part.groundAnchor==int(index)){part.bottom+=base;part.top+=base;}
            // Small foundation columns follow the actual slope, eliminating air
            // beneath a level building without flattening or changing the river.
            for(float dy=0;dy<anchor.length;dy+=1)
                for(float dx=0;dx<anchor.width;dx+=1)
                {
                    const float width=std::min(1.0f,anchor.width-dx),length=std::min(1.0f,anchor.length-dy);
                    const float sampleX=mirrored?frame.width-anchor.x-dx-width:anchor.x+dx;
                    float low=base;
                    for(float oy:{0.0f,length})for(float ox:{0.0f,width})
                        low=std::min(low,Height(game,x+sampleX+ox,worldY+anchor.forward+dy+oy));
                    if(base-low>0.01f)
                        result.boxes.push_back({anchor.x+dx,anchor.forward+dy,width,length,low-0.25f,base,anchor.foundation});
                }
        }
        return result;
    }
}
