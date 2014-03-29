#include "opengl_renderer.h"

#include <cassert>

#include <engine/render_sprite.h>

#include <foundation/memory.h>
#include <foundation/matrix4.h>

#include "gl3w.h"

namespace bowtie
{

OpenGLRenderer::OpenGLRenderer(Allocator& allocator) : Renderer(allocator)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
	set_active(false);
	notify_command_queue_populated();
	_rendering_thread.join();
}

GLuint compile_glsl_shader(const char* shader_source, GLenum shader_type)
{
	GLuint result = glCreateShader(shader_type);

	if(!result)
		return result;

	glShaderSource(result, 1, &shader_source, NULL);
	glCompileShader(result);

	GLint status = 0;
	glGetShaderiv(result, GL_COMPILE_STATUS, &status);

	assert(status && "Compilation of shader failed.");
	
	return result;
}
	
GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders)
{
    int i;
	GLuint program;

    program = glCreateProgram();

    for(i = 0; i < shader_count; i++)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

	GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    assert(status && "Failed linking shader program");

	if(delete_shaders)
	{
		for(i = 0; i < shader_count; i++)
			glDeleteShader(shaders[i]);
	}
	
	assert(glIsProgram(program));
	
	glValidateProgram(program);
	GLint validation_status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);
	assert(validation_status && "Failed to validate program");
	
    return program;
}

void OpenGLRenderer::test_draw(const View& view, ResourceHandle test_sprite_handle)
{	
	auto view_projection = view.view_projection();
		
	GLuint program = lookup_resource_object(1).render_handle;
	RenderSprite& test_sprite = *(RenderSprite*)lookup_resource_object(test_sprite_handle.handle).render_object;
	GLuint test_sprite_image = lookup_resource_object(test_sprite.image.handle).render_handle;

	auto test_image_scale_matrix = Matrix4();
	
	test_image_scale_matrix[0][0] = 128;
	test_image_scale_matrix[1][1] = 128;

	auto model_matrix = test_sprite.model * test_image_scale_matrix;
	auto model_view_projection_matrix = model_matrix * view_projection;

	assert(glIsProgram(program) && "Invalid shader program");
	glUseProgram(program);

	GLuint model_view_projection_matrix_id = glGetUniformLocation(program, "model_view_projection_matrix");
	glUniformMatrix4fv(model_view_projection_matrix_id, 1, GL_FALSE, &model_view_projection_matrix[0][0]);
		
	GLuint texture_sampler_id = glGetUniformLocation(program, "texture_sampler");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 1);
	glUniform1i(texture_sampler_id, test_sprite_image);

	auto sprite_rendering_quad = _resource_lut[_sprite_rendering_quad_handle.handle].render_handle;

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, sprite_rendering_quad);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0 
	);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
}

void OpenGLRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::flip()
{
	_context->flip();
}

void OpenGLRenderer::resize(const Vector2u& resolution)
{
	glViewport(0, 0, resolution.x, resolution.y);
}

RenderResourceHandle OpenGLRenderer::set_up_sprite_rendering_quad()
{
	static const GLfloat sprite_rendering_quad_vertices[] = {
	   0.0f, 0.0f, 0.0f,
	   1.0f, 0.0f, 0.0f,
	   0.0f, 1.0f, 0.0f,

	   1.0f, 0.0f, 0.0f,
	   1.0f, 1.0f, 0.0f,
	   0.0f, 1.0f, 0.0f
	};
	
	static GLuint sprite_rendering_quad_buffer;
	glGenBuffers(1, &sprite_rendering_quad_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sprite_rendering_quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_rendering_quad_vertices), sprite_rendering_quad_vertices, GL_STATIC_DRAW);

	return RenderResourceHandle(sprite_rendering_quad_buffer);
}

void OpenGLRenderer::run_thread()
{
	_context->make_current_for_calling_thread();

	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	// 2D needs no depth test.
	glDisable(GL_DEPTH_TEST);

	set_active(true);

	while (active())
	{
		{
			std::unique_lock<std::mutex> command_queue_populated_lock(_command_queue_populated_mutex);
			_wait_for_command_queue_populated.wait(command_queue_populated_lock, [&]{return _command_queue_populated;});
			_command_queue_populated = false;
		}

		consume_command_queue();
	}
}

RenderResourceHandle OpenGLRenderer::load_shader(ShaderResourceData& shader_data, void* dynamic_data)
{
	GLuint vertex_shader = compile_glsl_shader((char*)memory::pointer_add(dynamic_data, shader_data.vertex_shader_source_offset), GL_VERTEX_SHADER);
	GLuint fragment_shader = compile_glsl_shader((char*)memory::pointer_add(dynamic_data, shader_data.fragment_shader_source_offset), GL_FRAGMENT_SHADER);
		
	assert(vertex_shader != 0 && "Failed compiling vertex shader");
	assert(fragment_shader != 0 && "Failed compiling fragments shader");

	GLuint shaders[] = { vertex_shader, fragment_shader	};
	
	GLuint program = link_glsl_program(shaders, 2, true);
	
	if (program == 0)
	{
		char buf[1000];
		int len;
		glGetShaderInfoLog(program, 1000, &len, buf);
		printf("%s", buf);
	}

	assert(program != 0 && "Failed to link glsl shader");
	
	return program;
}

RenderResourceHandle OpenGLRenderer::load_BMP(TextureResourceData& trd, void* dynamic_data)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, trd.width, trd.height, 0, GL_BGR, GL_UNSIGNED_BYTE, memory::pointer_add(dynamic_data, trd.texture_data_dynamic_data_offset));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	return texture_id;
}

}
