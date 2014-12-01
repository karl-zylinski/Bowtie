#include "world.h"

#include <cassert>
#include <foundation/array.h>
#include <foundation/quad.h>
#include "drawable.h"
#include "material.h"
#include "render_interface.h"
#include "resource_manager.h"
#include "rectangle_geometry.h"
#include "sprite_geometry.h"
#include "text_geometry.h"

namespace bowtie
{

World::World(Allocator& allocator, RenderInterface& render_interface, ResourceManager& resource_manager) :
	_allocator(allocator), _drawables(array::create<Drawable*>(allocator)), _render_interface(render_interface), _resource_manager(resource_manager)
{
	rectangle_renderer_component::init(_rectangle_renderer_components, allocator);
	transform_component::init(_transform_components, allocator);
}

World::~World()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
		_allocator.destroy(_drawables[i]);

	array::deinit(_drawables);
	rectangle_renderer_component::deinit(_rectangle_renderer_components, _allocator);
	transform_component::deinit(_transform_components, _allocator);
}

void World::set_render_handle(RenderResourceHandle render_handle)
{
	_render_handle = render_handle;
}

Drawable* World::spawn_rectangle(const Rect& rect, const Color& color, int depth)
{
	auto geometry = _allocator.construct<RectangleGeometry>(color, rect);
	auto material = _resource_manager.load(ResourceType::Material, "shared/default_resources/rect.material");
	auto rectangle = _allocator.construct<Drawable>(_allocator, *geometry, (Material*)material.object, depth);
	rectangle->set_position(rect.position);
	array::push_back(_drawables, rectangle);
	_render_interface.spawn(*this, *rectangle, _resource_manager);
	return rectangle;
}

Drawable* World::spawn_sprite(const char* sprite_name, int depth)
{
	auto sprite_prototype = _resource_manager.get<Drawable>(ResourceType::Drawable, sprite_name);
	auto sprite = _allocator.construct<Drawable>(*sprite_prototype);
	sprite->set_depth(depth);
	array::push_back(_drawables, sprite);
	_render_interface.spawn(*this, *sprite, _resource_manager);

	return sprite;
}

void World::unspawn(Drawable& drawable)
{
	array::remove(_drawables, &drawable);
	_render_interface.unspawn(*this, drawable);
	_allocator.destroy(&drawable);
}

RenderResourceHandle World::render_handle()
{
	return _render_handle;
}

RectangleRendererComponent& World::rectangle_renderer_components()
{
	return _rectangle_renderer_components;
}

TransformComponent& World::transform_components()
{
	return _transform_components;
}

const Array<Drawable*>& World::drawables() const
{
	return _drawables;
}

void update_drawable_state(Allocator& allocator, RenderInterface& render_interface, Drawable& drawable)
{
	auto state_changed_command = render_interface.create_command(RendererCommand::DrawableStateReflection);
		
	auto& scd = *(DrawableStateReflectionData*)allocator.allocate(sizeof(DrawableStateReflectionData));
	scd.model = drawable.model_matrix();
	scd.material = drawable.material()->render_handle;
	scd.drawble = drawable.render_handle();
	scd.depth = drawable.depth();
	state_changed_command.data = &scd;

	drawable.reset_state_changed();

	render_interface.dispatch(state_changed_command);
}

void update_drawable_geometry(Allocator& allocator, RenderInterface& render_interface, Drawable& drawable)
{
	auto geometry_changed_command = render_interface.create_command(RendererCommand::DrawableGeometryReflection);
		
	auto& sgr = *(DrawableGeometryReflectionData*)allocator.allocate(sizeof(DrawableGeometryReflectionData));
	sgr.drawable = drawable.render_handle();
	sgr.size = drawable.geometry().data_size();
	
	geometry_changed_command.data = &sgr;
	geometry_changed_command.dynamic_data_size = sgr.size;
	geometry_changed_command.dynamic_data = allocator.allocate(sgr.size);
	memcpy(geometry_changed_command.dynamic_data, drawable.geometry().data(), sgr.size);

	drawable.reset_geometry_changed();

	render_interface.dispatch(geometry_changed_command);
}

Matrix4 world_matrix(const TransformComponentData& c, unsigned i)
{
	auto p = Matrix4();
	p[3][0] = -c.pivot[i].x;
	p[3][1] = -c.pivot[i].y;

	auto t = Matrix4();
	t[0][0] = cos(c.rotation[i]);
	t[1][0] = -sin(c.rotation[i]);
	t[0][1] = sin(c.rotation[i]);
	t[1][1] = cos(c.rotation[i]);
	t[3][0] = c.position[i].x;
	t[3][1] = c.position[i].y;

	auto parent_index = c.parent[i];

	if (parent_index == transform_component::not_assigned)
		return p * t;
	else
		return p * t * c.world_transform[parent_index];
}

void update_transforms(TransformComponentData& transform, unsigned start, unsigned end, RectangleRendererComponent& rectangle_renderer)
{
	for (unsigned i = start; i < end; ++i)
	{
		auto entity = transform.entity[i];
		auto world_transform = world_matrix(transform, i);
		transform.world_transform[i] = world_transform;

		if (!component::has_entity(rectangle_renderer.header, entity))
			continue;

		auto rectangle_index = hash::get(rectangle_renderer.header.map, transform.entity[i]);
		auto rect = rectangle_renderer.data.rect[rectangle_index];
		auto v1 = world_transform * Vector4(rect.position.x, rect.position.y, 0, 1);
		auto v2 = world_transform * Vector4(rect.position.x + rect.size.x, rect.position.y, 0, 1);
		auto v3 = world_transform * Vector4(rect.position.x, rect.position.y + rect.size.y, 0, 1);
		auto v4 = world_transform * Vector4(rect.position.x + rect.size.x, rect.position.y + rect.size.y, 0, 1);

		Quad geometry = {
			Vector2(v1.x, v1.y),
			Vector2(v2.x, v2.y),
			Vector2(v3.x, v3.y),
			Vector2(v4.x, v4.y)
		};

		rectangle_renderer_component::set_geometry(rectangle_renderer, entity, geometry);
	}
}

void create_rectangles(Allocator& allocator, RenderInterface& ri, RenderResourceHandle default_material, RenderResourceHandle render_world, RectangleRendererComponent& rectangle_renderer, unsigned num)
{
	auto rrd = ri.create_render_resource_data(RenderResourceData::RectangleRenderer);

	for (unsigned i = rectangle_renderer.header.first_new; i < rectangle_renderer.header.num; ++i)
	{
		rectangle_renderer.data.render_handle[i] = ri.create_handle();
		rectangle_renderer.data.material[i] = default_material;
	}

	CreateRectangleRendererData data;
	data.num = num;
	data.world = render_world;
	rrd.data = &data;
	ri.create_resource(rrd, rectangle_renderer_component::copy_new_data(rectangle_renderer, allocator), rectangle_renderer_component::component_size * data.num);
}

void update_rectangles(Allocator& allocator, RenderInterface& ri, RectangleRendererComponent& rectangle_renderer, unsigned num)
{
	auto rrd = ri.create_render_resource_data(RenderResourceData::RectangleRenderer);
	UpdateRectangleRendererData data;
	data.num = num;
	rrd.data = &data;
	ri.update_resource(rrd, rectangle_renderer_component::copy_dirty_data(rectangle_renderer, allocator), rectangle_renderer_component::component_size * data.num);
}

void World::update()
{
	for (unsigned i = 0; i < array::size(_drawables); ++i)
	{
		auto drawable = _drawables[i];
		
		if (drawable->state_changed())
			update_drawable_state(_allocator, _render_interface, *drawable);
		
		if (drawable->geometry_changed())
			update_drawable_geometry(_allocator, _render_interface, *drawable);
	}

	{
		if (component::num_new(_transform_components.header) > 0)
			update_transforms(_transform_components.data, _transform_components.header.first_new, _transform_components.header.num, _rectangle_renderer_components);
	
		component::reset_new(_transform_components.header);	
	}

	{
		const auto num_dirty_transforms = component::num_dirty(_transform_components.header);

		if (num_dirty_transforms > 0)
			update_transforms(_transform_components.data, 0, num_dirty_transforms, _rectangle_renderer_components);

		component::reset_dirty(_transform_components.header);
	}
	
	{
		const auto num_new_rectangles = component::num_new(_rectangle_renderer_components.header);

		if (num_new_rectangles > 0)
		{
			auto default_material = ((Material*)_resource_manager.load(ResourceType::Material, "shared/default_resources/rect.material").object)->render_handle;
			create_rectangles(_allocator, _render_interface, default_material, _render_handle, _rectangle_renderer_components, num_new_rectangles);
		}

		component::reset_new(_rectangle_renderer_components.header);
	}

	{
		const auto num_dirty_rectangles = component::num_dirty(_rectangle_renderer_components.header);

		if (num_dirty_rectangles > 0)
			update_rectangles(_allocator, _render_interface, _rectangle_renderer_components, num_dirty_rectangles);
	
		component::reset_dirty(_rectangle_renderer_components.header);
	}
}

void World::draw(const Rect& view)
{
	auto render_world_command = _render_interface.create_command(RendererCommand::RenderWorld);

	auto& rwd = *(RenderWorldData*)_allocator.allocate(sizeof(RenderWorldData));
	rwd.view = view;
	rwd.render_world = _render_handle;
	render_world_command.data = &rwd;

	_render_interface.dispatch(render_world_command);
}

Drawable* World::spawn_text(const Font& font, const char* text_str, int depth)
{
	auto text_geometry = _allocator.construct<TextGeometry>(font, _allocator);
	text_geometry->set_text(text_str);
	auto text = _allocator.construct<Drawable>(_allocator, *text_geometry, (Material*)_resource_manager.get_default(ResourceType::Material).object, depth);
	array::push_back(_drawables, text);
	_render_interface.spawn(*this, *text, _resource_manager);
	return text;
}

} // namespace bowtie
