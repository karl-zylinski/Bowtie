#include "engine.h"

#include <cmath>
#include <cstring>
#include <stdio.h>

#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/string_utils.h>

#include "render_interface.h"
#include "renderer_command.h"
#include "render_resource_types.h"
#include "timer.h"
#include "world.h"

namespace bowtie
{

Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface),
	_resource_manager(allocator, render_interface), _game(allocator, *this)
{
	timer::start();
}

Engine::~Engine()
{
	if (_game.initialized())
		_game.deinit();
}

void Engine::update()
{
	if (!_render_interface.is_setup())
		return;

	if (!_game.initialized())
		_game.init();
				
	float time_elapsed = timer::counter();
	float dt = time_elapsed - _time_elapsed_previous_frame;
	_time_elapsed_previous_frame = time_elapsed;

	_time_since_start += dt;

	_render_interface.wait_for_fence(_render_interface.create_fence());

	_game.update(dt);
	_game.draw();
	
	_render_interface.dispatch(_render_interface.create_command(RendererCommand::CombineRenderedWorlds));

	_keyboard = Keyboard::from_previous_frame(_keyboard);
}

void Engine::resize(const Vector2u& resolution)
{
	_render_interface.resize(resolution);
}

RenderInterface& Engine::render_interface()
{
	return _render_interface;
}

World* Engine::create_world()
{
	auto world = MAKE_NEW(_allocator, World, _allocator, _render_interface, _resource_manager);
	_render_interface.create_render_world(*world);
	return world;
}

void Engine::destroy_world(World& world)
{
	MAKE_DELETE(_allocator, World, &world);
}

void Engine::key_pressed(keyboard::Key key)
{
	_keyboard.set_key_pressed(key);
}

void Engine::key_released(keyboard::Key key)
{
	_keyboard.set_key_released(key);
}

const Keyboard& Engine::keyboard() const
{
	return _keyboard;
}

ResourceManager& Engine::resource_manager()
{
	return _resource_manager;
}

}