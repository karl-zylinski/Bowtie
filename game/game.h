#pragma once

namespace bowtie
{

struct Allocator;
struct Engine;
struct RenderInterface;

struct Game
{
    bool started;
};

namespace game
{
    void init(Game* g, Allocator* allocator, Engine* engine, RenderInterface* render_interface);
    void start(Game* g);
    void deinit(Game* g);
    void update(Game* g, real32 dt);
    void draw(Game* g);
    void deinit(Game* g);
}

} // namespace bowtie
