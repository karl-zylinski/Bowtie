#pragma once

#include <foundation/collection_types.h>
#include "render_resource_handle.h"
#include "entity/components/sprite_renderer_component.h"
#include "entity/components/transform_component.h"

namespace bowtie
{

typedef unsigned Entity;
struct Vector4;
struct Font;
struct Rect;
struct Material;
struct RenderInterface;
class ResourceManager;

struct World
{
	Allocator* allocator;
	RenderResourceHandle render_handle;
	RenderInterface* render_interface;
	RenderResourceHandle default_material;
	TransformComponent transform_components;
	SpriteRendererComponent sprite_renderer_components;
};

namespace world
{
	void init(World& w, Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager);
	void deinit(World& w);
	void update(World& w);
	void draw(World& w, const Rect& view);
}

};
