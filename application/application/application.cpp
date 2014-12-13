#include <engine/engine.h>
#include <engine/resource_store.h>
#include <foundation/array.h>
#include <foundation/malloc_allocator.h>
#include <foundation/memory.h>
#include <opengl_renderer/opengl_context_windows.h>
#include <opengl_renderer/opengl_renderer.h>
#include <os/windows/callstack_capturer.h>
#include <os/windows/window.h>
#include <renderer/renderer.h>
#include <engine/timer.h>
#include <os/windows/timer.h>

using namespace bowtie;

namespace
{
	Engine* s_engine;
	Renderer* s_renderer;
	OpenGLContextWindows* s_context;
	Allocator* s_allocator;
}

void create_render_context_callback(HWND hwnd, const Vector2u* resolution)
{
	s_context->create(hwnd);
	renderer::run(*s_renderer, s_context, *resolution);
}

void window_resized_callback(const Vector2u* resolution)
{
	engine::resize(s_engine, resolution);
}

void key_down_callback(platform::Key key)
{
	engine::key_pressed(s_engine, key);
}

void key_up_callback(platform::Key key)
{
	engine::key_released(s_engine, key);
}

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	auto callstack_capturer = windows::callstack_capturer::create();
	auto& allocator = *(new MallocAllocator());
	memory::init_allocator(allocator, "default allocator", &callstack_capturer);
	s_allocator = &allocator;
	auto& renderer_allocator = *(new MallocAllocator());
	memory::init_allocator(renderer_allocator, "renederer allocator", &callstack_capturer);

	{
		ConcreteRenderer opengl_renderer = opengl_renderer::create();
		Renderer renderer;
		renderer::init(renderer, opengl_renderer, renderer_allocator, allocator);
		s_renderer = &renderer;
		OpenGLContextWindows context;
		s_context = &context;
		auto& render_interface = renderer.render_interface;

		{
			Timer timer = {};
			timer.counter = windows::timer::counter;
			timer.start = windows::timer::start;
			Engine engine = {};
			engine::init(&engine, &allocator, &render_interface, &timer);
			s_engine = &engine;
			auto resolution = Vector2u(1280, 720);
			windows::Window window = {};
			windows::window::init(&window, instance, &resolution, &create_render_context_callback, &window_resized_callback, &key_down_callback, &key_up_callback);	
				
			while(window.is_open)
			{
				windows::window::dispatch_messages(&window);
				engine::update_and_render(&engine);
				renderer::deallocate_processed_commands(renderer, allocator);
			}

			engine::deinit(&engine);
		}
		
		renderer::stop(renderer, allocator);
		renderer::deinit(renderer);
	}

	memory::deinit_allocator(renderer_allocator);
	memory::deinit_allocator(allocator);
}
