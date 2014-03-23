#include <stdio.h>

#include <engine/engine.h>
#include <foundation/array.h>
#include <foundation/temp_allocator.h>
#include <opengl_renderer/opengl_context_windows.h>
#include <opengl_renderer/opengl_renderer.h>
#include <os/windows/window.h>

using namespace bowtie;

namespace
{
	Engine* s_engine;
	OpenGLRenderer* s_renderer;
	OpenGLContextWindows* s_context;
	Allocator* s_allocator;
}

void create_render_context_callback(HWND hwnd, const Vector2u& resolution)
{
	s_context->create(hwnd);
	s_renderer->set_opengl_context(s_context);
	s_renderer->run_render_thread();
	s_renderer->resize(resolution);
}

void window_resized_callback(const Vector2u& resolution)
{
	s_engine->resize(resolution);
}

void key_down_callback(keyboard::Key)
{
}

void key_up_callback(keyboard::Key)
{
}

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	memory_globals::init();
	Allocator& allocator = memory_globals::default_allocator();
	s_allocator = &allocator;

	{
		OpenGLRenderer renderer(allocator);
		s_renderer = &renderer;

		OpenGLContextWindows context;
		s_context = &context;

		Engine engine(allocator, renderer.render_interface());
		s_engine = &engine;

		auto resolution = Vector2u(640, 480);
		Window window(instance, resolution, &create_render_context_callback, &window_resized_callback,
			&key_down_callback, &key_up_callback);
		
		while(window.is_open())
		{			
			window.dispatch_messages();
			engine.update();
		}
	}

	memory_globals::shutdown();
}
