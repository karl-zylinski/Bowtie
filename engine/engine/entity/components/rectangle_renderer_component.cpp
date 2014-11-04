#pragma once

#include "rectangle_renderer_component.h"
#include "../../rect.h"
#include "../../material.h"
#include <foundation/vector4.h>
#include <cassert>

namespace bowtie
{

namespace
{

RectangleRendererComponentData initialize_data(void* buffer, unsigned size)
{
	RectangleRendererComponentData new_data;
	new_data.color = (Color*)buffer;
	new_data.rect = (Rect*)(new_data.color + size);
	new_data.material = (RenderResourceHandle*)(new_data.rect + size);
	new_data.render_handle = (RenderResourceHandle*)(new_data.material + size);
	return new_data;
}

void move_one(RectangleRendererComponent& c, unsigned from, unsigned to)
{
	c.data.color[to] = c.data.color[from];
	c.data.rect[to] = c.data.rect[from];
	c.data.material[to] = c.data.material[from];
	c.data.render_handle[to] = c.data.render_handle[from];
}

void copy(RectangleRendererComponent& c, RectangleRendererComponentData& dest, unsigned num)
{
	memcpy(dest.color, c.data.color, num * sizeof(Color));
	memcpy(dest.rect, c.data.rect, num * sizeof(Rect));
	memcpy(dest.material, c.data.material, num * sizeof(Material));
	memcpy(dest.render_handle, c.data.render_handle, num * sizeof(RenderResourceHandle));
}

void copy(RectangleRendererComponent& c, RectangleRendererComponentData& dest)
{
	copy(c, dest, c.num);
}

void grow(RectangleRendererComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = c.capacity == 0 ? 8 : c.capacity * 2;
	const unsigned bytes = new_capacity * rectangle_renderer_component::component_size;
	void* buffer = allocator.allocate(bytes);

	auto new_data = initialize_data(buffer, new_capacity);
	copy(c, new_data);
	c.data = new_data;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.capacity = new_capacity;
}

void mark_dirty(RectangleRendererComponent& c, Entity e)
{
	auto entity_index = hash::get(c.map, e, 0u);

	if (c.last_dirty_index != (unsigned)-1 && entity_index <= c.last_dirty_index)
		return;

	auto current_dirty_index = ++c.last_dirty_index;

	if (current_dirty_index == entity_index)
		return;

	auto color_at_index = c.data.color[current_dirty_index];
	auto rect_at_index = c.data.rect[current_dirty_index];
	auto material_at_index = c.data.material[current_dirty_index];
	auto render_handle_at_index = c.data.render_handle[current_dirty_index];
	move_one(c, entity_index, current_dirty_index);
	c.data.color[entity_index] = color_at_index;
	c.data.rect[entity_index] = rect_at_index;
	c.data.material[entity_index] = material_at_index;
	c.data.render_handle[entity_index] = render_handle_at_index;
}

}

namespace rectangle_renderer_component
{

unsigned component_size = (sizeof(Color) + sizeof(Rect) + sizeof(RenderResourceHandle) + sizeof(RenderResourceHandle));

void init(RectangleRendererComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(RectangleRendererComponent));
	c.map = hash::create<unsigned>(allocator);
	c.last_dirty_index = (unsigned)-1;
}

void deinit(RectangleRendererComponent& c, Allocator& allocator)
{
	hash::deinit(c.map);
	allocator.deallocate(c.buffer);
}

void create(RectangleRendererComponent& c, Entity e, Allocator& allocator)
{
	if (c.num >= c.capacity)
		grow(c, allocator);

	unsigned i = c.num++;
	hash::set(c.map, e, i);
	c.data.color[i] = Color(1, 1, 1, 1);
	c.data.rect[i] = Rect();
	c.data.material[i] = RenderResourceHandle::NotInitialized;
	c.data.render_handle[i] = RenderResourceHandle::NotInitialized;
}

void destroy(RectangleRendererComponent& c, Entity e)
{
	unsigned i = hash::get(c.map, e, 0u);
	hash::remove(c.map, e);
	--c.num;

	if (i == c.num)
		return;
		
	move_one(c, c.num, i);
}

void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect)
{	
	c.data.rect[hash::get(c.map, e)] = rect;
	mark_dirty(c, e);
}

const Rect& rect(RectangleRendererComponent& c, Entity e)
{
	return c.data.rect[hash::get(c.map, e)];
}

void set_color(RectangleRendererComponent& c, Entity e, const Color& color)
{
	c.data.color[hash::get(c.map, e)] = color;
	mark_dirty(c, e);
}

const Color& color(RectangleRendererComponent& c, Entity e)
{
	return c.data.color[hash::get(c.map, e)];
}

void set_render_handle(RectangleRendererComponent& c, Entity e, RenderResourceHandle render_handle)
{
	c.data.render_handle[hash::get(c.map, e)] = render_handle;
}

RenderResourceHandle material(RectangleRendererComponent& c, Entity e)
{
	return c.data.material[hash::get(c.map, e)];
}

void set_material(RectangleRendererComponent& c, Entity e, RenderResourceHandle material)
{
	c.data.material[hash::get(c.map, e)] = material;
}

RenderResourceHandle render_handle(RectangleRendererComponent& c, Entity e)
{
	return c.data.render_handle[hash::get(c.map, e)];
}

RectangleRendererComponentData* copy_data(RectangleRendererComponent& c, Entity e, Allocator& allocator)
{
	RectangleRendererComponentData* data = (RectangleRendererComponentData*)allocator.allocate(sizeof(RectangleRendererComponentData));
	*data = initialize_data(allocator.allocate(component_size), 1);
	*data->color = color(c, e);
	*data->rect = rect(c, e);
	*data->material = material(c, e);
	*data->render_handle = render_handle(c, e);
	return data;
}

RectangleRendererComponentData* copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator)
{
	RectangleRendererComponentData* data = (RectangleRendererComponentData*)allocator.allocate(sizeof(RectangleRendererComponentData));
	auto num_dirty = c.last_dirty_index + 1;
	*data = initialize_data(allocator.allocate(component_size), num_dirty);
	copy(c, *data, num_dirty);
	return data;
}

} // rectangle_renderer_component

} // namespace bowtie