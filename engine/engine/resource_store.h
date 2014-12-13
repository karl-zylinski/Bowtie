#pragma once

#include <cassert>
#include <stdint.h>

#include <foundation/hash.h>
#include <foundation/murmur_hash.h>
#include <foundation/string_utils.h>

#include "render_resource_handle.h"
#include "resource_type.h"

namespace bowtie
{
struct Material;
struct RenderInterface;
struct Texture;
struct Image;
struct Shader;
struct Font;

struct ResourceStore
{
	Allocator* allocator;
	RenderInterface* render_interface;
	Hash<void*> _resources;
	Option<void*> _default_resources[(unsigned)ResourceType::NumResourceTypes];
};

namespace resource_store
{
	static const char* resource_type_names[] = { "not_initialized", "shader", "image", "texture", "font", "material" };
	ResourceType resource_type_from_string(const char* type);

	void init(ResourceStore* rs, Allocator* allocator, RenderInterface* render_interface);
	void deinit(ResourceStore* rs);
	Option<void*> load(ResourceStore* rs, ResourceType type, const char* filename);
	Option<void*> get(const ResourceStore* rs, ResourceType type, uint64_t name);
	void reload(ResourceStore* rs, ResourceType type, const char* filename);
	void reload_all(ResourceStore* rs);
	void set_default(ResourceStore* rs, ResourceType type, void* resource);
	Option<void*> get_default(const ResourceStore* rs, ResourceType type);
}

} // namespace bowtie