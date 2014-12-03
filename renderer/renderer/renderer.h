#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <engine/irenderer.h>
#include <engine/renderer_command.h>
#include <engine/render_interface.h>
#include <engine/render_resource_types.h>
#include <foundation/collection_types.h>

#include "render_resource_table.h"
#include "render_resource.h"
#include "irenderer_context.h"
#include "render_world.h"

namespace bowtie
{

struct ConcreteRenderer;
struct RenderTarget;

struct RendererResourceObject
{
	RendererResourceObject(RenderResourceData::Type type, RenderResourceHandle handle)
		: type(type), handle(handle) {}

	RenderResourceData::Type type;
	RenderResourceHandle handle;
};

struct CreatedResources {
	unsigned num;
	RenderResourceHandle* handles;
	RenderResource* resources;
};

struct UpdatedResources {
	unsigned num;
	RenderResourceHandle* handles;
	RenderResource* old_resources;
	RenderResource* new_resources;
};

class Renderer : public IRenderer
{
public:
	Renderer(ConcreteRenderer& concrete_renderer_obj, Allocator& renderer_allocator, Allocator& render_interface_allocator);
	~Renderer();

	typedef std::function<RenderResource(RenderResourceHandle)> LookupResourceFunction;
	
	void add_renderer_command(const RendererCommand& command);
	RenderResourceHandle create_handle();
	void deallocate_processed_commands(Allocator& render_interface_allocator);
	void free_handle(RenderResourceHandle handle);
	bool is_active() const;
	bool is_setup() const;
	RenderInterface& render_interface();
	const Vector2u& resolution() const;
	void run(IRendererContext* context, const Vector2u& resolution);
	void stop(Allocator& render_interface_allocator);
	
private:
	void consume_command_queue();
	CreatedResources create_resources(RenderResourceData::Type type, void* data, void* dynamic_data);
	void execute_command(const RendererCommand& command);
	void notify_unprocessed_commands_exists();
	void thread();
	UpdatedResources update_resources(RenderResourceData::Type type, void* data, void* dynamic_data);
	void wait_for_unprocessed_commands_to_exist();
	
	bool _active;
	Allocator& _allocator;
	Array<RendererCommand> _command_queue;
	bool _unprocessed_commands_exists;
	std::mutex _unprocessed_commands_exists_mutex;
	ConcreteRenderer& _concrete_renderer;
	IRendererContext* _context;
	Array<RenderResourceHandle> _free_handles;
	Array<void*> _processed_memory;
	std::mutex _processed_memory_mutex;
	RenderInterface _render_interface;
	Vector2u _resolution;
	RenderResource _resource_table[render_resource_table::size];
	Array<RenderTarget> _render_targets;
	Array<RenderWorld*> _rendered_worlds; // filled each frame with all rendered world, in order
	Array<RendererResourceObject> _resource_objects;
	RenderResource _fullscreen_rendering_quad;
	RenderResource _rendered_worlds_combining_shader;
	bool _setup;
	bool _shut_down;
	std::thread _thread;
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	std::condition_variable _wait_for_unprocessed_commands_to_exist;
};

}
