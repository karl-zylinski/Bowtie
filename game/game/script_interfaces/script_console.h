#pragma once

struct lua_State;

namespace bowtie
{

namespace console
{

void init(lua_State* lua);
void write(const char* message);
void update(float dt);
void draw();

}

}