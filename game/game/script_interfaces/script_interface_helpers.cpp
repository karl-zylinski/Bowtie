#include "script_interface_helpers.h"

#include <cassert>

#include <lua.hpp>

namespace bowtie
{
namespace script_interface
{

void register_interface(lua_State* lua, const char* interface_name, const interface_function* functions, unsigned num_functions)
{
	lua_newtable(lua);

	for (unsigned i = 0; i < num_functions; ++i)
	{
		auto func = functions[i];

		lua_pushcfunction(lua, func.function);
		lua_setfield(lua, -2, func.name);
	}

	lua_setglobal(lua, interface_name);
}

double get_field (lua_State* lua, int index, const char *key) {
    lua_pushstring(lua, key);
    lua_gettable(lua, index);

    if (!lua_isnumber(lua, -1))
		return 0;

    auto result = lua_tonumber(lua, -1);
    lua_pop(lua, 1);
    return result;
}

Vector2 to_vector(lua_State* lua, int index)
{
	assert(lua_istable(lua, index));

	auto x_value = (float)get_field(lua, index, "x");
	auto y_value = (float)get_field(lua, index, "y");

	return Vector2(x_value, y_value);
}

void push_vector(lua_State* lua, const Vector2& v)
{
	lua_newtable(lua);
	lua_pushnumber(lua, v.x);
	lua_setfield(lua, -2, "x");
	lua_pushnumber(lua, v.y);
	lua_setfield(lua, -2, "y");
}

} // namespace script_interface
} // namespace bowtie