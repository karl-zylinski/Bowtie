#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <engine/irenderer.h>
#include <engine/renderer_command.h>
#include <engine/render_interface.h>
#include <engine/render_resource_types.h>
#include <foundation/collection_types.h>

#include "render_resource_lookup_table.h"
#include "render_resource_handle.h"
#include "renderer_context.h"
#include "render_world.h"

namespace bowtie
{

class IConcreteRenderer;
struct RenderDrawable;
struct RenderTarget;

struct RendererResourceObject
{
	RendererResourceObject(RenderResourceData::Type, RenderResourceHandle handle)
		: type(type), handle(handle) {}

	RenderResourceData::Type type;
	RenderResourceHandle handle;
};

class Renderer : public IRenderer
{
public:
	Renderer(IConcreteRenderer& concrete_renderer, Allocator& renderer_allocator, Allocator& render_interface_allocator, RenderResourceLookupTable& render_resource_lookup_table);
	~Renderer();

	typedef std::function<RenderResourceHandle(ResourceHandle)> LookupResourceFunction;
	
	void add_renderer_command(const RendererCommand& command);
	ResourceHandle create_handle();
	void deallocate_processed_commands(Allocator& allocator);
	bool is_active() const;
	bool is_setup() const;
	RenderInterface& render_interface();
	const Vector2u& resolution() const;
	void run(RendererContext* context, const Vector2u& resolution);
	
private:
	static RenderResourceHandle create_drawable(Allocator& allocator, const DrawableResourceData& drawable_data);
	static RenderResourceHandle create_resource(Allocator& allocator, IConcreteRenderer& concrete_renderer, void* dynamic_data, const RenderResourceData& render_resource, Array<RenderTarget*>& render_targets, const RenderResourceLookupTable& resource_lut);
	static RenderResourceHandle create_world(Allocator& allocator, IConcreteRenderer& concrete_renderer);
	void consume_command_queue();
	void consume_create_resource(void* dynamic_data, const RenderResourceData& render_resource);
	static void drawable_state_reflection(RenderDrawable& drawable, const DrawableStateReflectionData& data);
	void execute_command(const RendererCommand& command);
	static void flip(RendererContext& context);
	static void move_processed_commads(Array<RendererCommand>& command_queue, Array<void*>& processed_memory, std::mutex& processed_memory_mutex);
	static void move_unprocessed_commands(Array<RendererCommand>& command_queue, Array<RendererCommand>& unprocessed_commands, std::mutex& unprocessed_commands_mutex);
	void notify_unprocessed_commands_consumed();
	void notify_unprocessed_commands_exists();
	static void raise_fence(RenderFence& fence);
	static void render_world(IConcreteRenderer& concrete_renderer, Array<RenderWorld*>& rendered_worlds, RenderWorld& render_world, const View& view);
	void thread();
	void wait_for_unprocessed_commands_to_exist();

	bool _active;
	Allocator& _allocator;
	Array<RendererCommand> _command_queue;
	bool _unprocessed_commands_exists;
	std::mutex _unprocessed_commands_exists_mutex;
	IConcreteRenderer& _concrete_renderer;
	RendererContext* _context;
	Array<ResourceHandle> _free_handles;
	Array<void*> _processed_memory;
	std::mutex _processed_memory_mutex;
	RenderInterface _render_interface;
	RenderResourceLookupTable& _resource_lut;
	Array<RenderTarget*> _render_targets;
	Array<RenderWorld*> _rendered_worlds; // filled each frame with all rendered world, in order
	Array<RendererResourceObject> _resource_objects;
	bool _setup;
	std::thread _thread;
	Array<RendererCommand> _unprocessed_commands;
	std::mutex _unprocessed_commands_mutex;
	std::condition_variable _wait_for_unprocessed_commands_to_exist;
};

}
