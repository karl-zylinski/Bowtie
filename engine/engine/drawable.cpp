#include "drawable.h"
#include <foundation/memory.h>
#include "idrawable_geometry.h"
#include <cmath>

namespace bowtie
{

////////////////////////////////
// Public interface.

Drawable::Drawable(Allocator& allocator, IDrawableGeometry& geometry, Material* material) : _allocator(allocator), _depth(0), _geometry(geometry),
	 _pivot(0, 0), _position(0, 0), _render_state_changed(false), _rotation(0.0f), _material(material)
{
}

Drawable::Drawable(const Drawable& other) : _allocator(other._allocator), _depth(0), _geometry(other._geometry.clone(_allocator)),
	_pivot(0, 0), _position(other._position), _render_state_changed(false), _rotation(0.0f), _material(other._material)
{
}

Drawable::~Drawable()
{
	_allocator.destroy(&_geometry);
}

const Color& Drawable::color() const
{
	return _geometry.color();
}

float Drawable::depth() const
{
	return _depth;
}

IDrawableGeometry& Drawable::geometry()
{
	return _geometry;
}

const IDrawableGeometry& Drawable::geometry() const
{
	return _geometry;
}

bool Drawable::geometry_changed() const
{
	return _geometry.has_changed();
}

RenderResourceHandle Drawable::geometry_handle() const
{
	return _geometry_handle;
}

Material* Drawable::material() const
{
	return _material;
}

Matrix4 Drawable::model_matrix() const
{
	auto p = Matrix4();	
	p[3][0] = (float)-_pivot.x;
	p[3][1] = (float)-_pivot.y;
	
	auto r = Matrix4();
	r[0][0] = cos(_rotation);
	r[1][0] = -sin(_rotation);
	r[0][1] = sin(_rotation);
	r[1][1] = cos(_rotation);
		
	auto t = Matrix4();
	t[3][0] = _position.x;
	t[3][1] = _position.y;
	t[3][2] = _depth;

	if (_pivot.x == 0 && _pivot.y == 0)
		return r * t;
	else
		return p * r * t;
}

const Vector2i& Drawable::pivot() const
{
	return _pivot;
}

const Vector2& Drawable::position() const
{
	return _position;
}

RenderResourceHandle Drawable::render_handle() const
{
	return _render_handle;
}

void Drawable::reset_geometry_changed()
{
	_geometry.reset_has_changed();
}

void Drawable::reset_state_changed()
{
	_render_state_changed = false;
}

float Drawable::rotation() const
{
	return _rotation;
}

void Drawable::set_color(const Color& color)
{
	_geometry.set_color(color);
}

void Drawable::set_depth(float depth)
{
	_depth = depth;
	_render_state_changed = true;
}

void Drawable::set_geometry_handle(RenderResourceHandle handle)
{
	_geometry_handle = handle;
}

void Drawable::set_material(Material* material)
{
	_material = material;
	_render_state_changed = true;
}

void Drawable::set_pivot(const Vector2i& pivot)
{
	_pivot = pivot;
	_render_state_changed = true;
}

void Drawable::set_position(const Vector2& position)
{
	_position = position;
	_render_state_changed = true;
}

void Drawable::set_render_handle(RenderResourceHandle handle)
{
	assert(_render_handle == RenderResourceHandle::NotInitialized && "Trying to reset already initliaized drawable render handle.");
	_render_handle = handle;
}

void Drawable::set_rotation(float rotation)
{
	_rotation = rotation;
	_render_state_changed = true;
}

bool Drawable::state_changed() const
{
	return _render_state_changed;
}

}
