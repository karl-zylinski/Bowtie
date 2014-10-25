#pragma once

#include <engine/render_resource_handle.h>

namespace bowtie
{

struct RenderResource;

namespace render_resource_table
{
	const unsigned size = 4000;
	void free(RenderResource* table, RenderResourceHandle handle);
	const RenderResource& lookup(const RenderResource* table, RenderResourceHandle handle);
	void set(RenderResource* table, RenderResourceHandle handle, const RenderResource& resource);
}

} // namespace bowtie;