#include "script_material.h"

#include <engine/render_interface.h>
#include <engine/resource_handle.h>
#include <foundation/murmur_hash.h>
#include <lua.hpp>
#include "script_interface_helpers.h"

namespace bowtie
{
namespace material_script_interface
{
	
namespace 
{
	RenderInterface* s_render_interface = nullptr;
}

int set_uniform_value(lua_State* lua)
{
	auto material = ResourceHandle((unsigned)lua_tonumber(lua, 1));
	auto name = hash_str(lua_tostring(lua, 2));
	auto value = script_interface::to_vector4(lua, 3);
	auto command = s_render_interface->create_command(RendererCommand::SetUniformValue);
	SetUniformValueData& suvd = *(SetUniformValueData*)command.data;
	suvd.material = material;
	suvd.uniform_name = name;
	suvd.value = value;
	command.data = &suvd;
	s_render_interface->dispatch(command);
	return 0;
}

void load(lua_State* lua, RenderInterface& render_interface)
{
	s_render_interface = &render_interface;

	const interface_function functions[] = {
		{ "set_uniform_value", set_uniform_value }
	};

	script_interface::register_interface(lua, "Material", functions, 1);
}

} // namespace drawable_script_interface
} // namespace bowtie