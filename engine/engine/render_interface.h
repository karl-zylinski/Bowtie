#pragma once

#include "renderer_command.h"
#include "render_resource_types.h"

namespace bowtie
{

class Allocator;
class IRenderer;
class World;
class Drawable;
struct Texture;
struct RenderFence;
class ResourceManager;
class RenderInterface
{
public:
	RenderInterface(IRenderer& renderer, Allocator& allocator);
	
	bool is_setup() const;
	bool is_active() const;
	
	void create_texture(Texture& texture);
	void spawn(World& world, Drawable& drawable, ResourceManager& resource_manager);
	void unspawn(World& world, Drawable& drawable);
	void create_render_world(World& world);
	RenderResourceData create_render_resource_data(RenderResourceData::Type type);
	RendererCommand create_command(RendererCommand::Type type);
	RenderResourceHandle create_handle();
	void dispatch(const RendererCommand& command, void* dynamic_data, unsigned dynamic_data_size);
	void dispatch(const RendererCommand& command);
	void create_resource(RenderResourceData& resource, void* dynamic_data = nullptr, unsigned dynamic_data_size = 0);
	void update_resource(RenderResourceData& resource, void* dynamic_data = nullptr, unsigned dynamic_data_size = 0);
						  
	// Pass the main thread allocator.
	void deallocate_processed_commands(Allocator& allocator);

	RenderFence& create_fence();
	void wait_for_fence(RenderFence& fence);
	void resize(const Vector2u& resolution);
	const Vector2u& resolution() const;

private:
	Allocator& _allocator;
	IRenderer& _renderer;
	
	RenderInterface(const RenderInterface&);
	RenderInterface& operator=(const RenderInterface&);
};

}