#include "render_world.h"
#include <foundation/array.h>
#include <engine/rect.h>
#include "render_target.h"
#include "render_component.h"
#include <algorithm>

namespace bowtie
{

namespace render_world
{

void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator)
{
	rw.components = array::create<RenderComponent*>(allocator);
	rw.render_target = render_target;
}

void deinit(RenderWorld& rw)
{
	array::deinit(rw.components);
}

void add_component(RenderWorld& rw, RenderComponent* component)
{
	array::push_back(rw.components, component);
}

void sort(RenderWorld& rw)
{
	std::sort(&rw.components[0], &rw.components[array::size(rw.components)], [](RenderComponent* x, RenderComponent* y){ return x->material < y->material; });
}

} // namespace render_world

} // namespace bowtie