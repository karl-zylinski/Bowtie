#pragma once

struct lua_State;

namespace bowtie
{

struct Engine;

namespace world_script_interface
{

void load(lua_State* lua, Engine* engine);

}
}