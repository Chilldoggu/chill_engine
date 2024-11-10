#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image/stb_image.h>

#include <iostream>

#include "chill_renderer/buffers.hpp"
#include "chill_renderer/file_manager.hpp"
#include "chill_renderer/application.hpp"

namespace chill_renderer { 
namespace fs = std::filesystem;

fs::path guess_path(const std::wstring& a_path) {
	// Guess project directory
	fs::path guessed_proj_dir = fs::current_path();
	if (guessed_proj_dir.filename() == "build" || guessed_proj_dir.filename() == "install") {
		guessed_proj_dir = guessed_proj_dir.parent_path();
	}

	// Check if path leads to a non-empty file.
	fs::path ret_path(a_path);
	if (!(fs::exists(ret_path) && fs::is_regular_file(ret_path) && !fs::is_empty(ret_path))) {
		ret_path = guessed_proj_dir / ret_path;
		if (!(fs::exists(ret_path) && fs::is_regular_file(ret_path) && !fs::is_empty(ret_path)))
			return fs::path("");
	}

	return fs::canonical(ret_path);
}

BufferObjects::BufferObjects(const BufferObjects& a_obj) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::MESHES, a_obj.VAO);

	VAO = a_obj.VAO;
	EBO = a_obj.EBO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
}

BufferObjects::BufferObjects(BufferObjects&& a_obj) noexcept {
	VAO = a_obj.VAO;
	EBO = a_obj.EBO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;

	a_obj.VAO = EMPTY_VBO;
	a_obj.VBO_UVs = EMPTY_VBO;
	a_obj.VBO_pos = EMPTY_VBO;
	a_obj.VBO_normals = EMPTY_VBO;
	a_obj.EBO = EMPTY_VBO;
}

BufferObjects& BufferObjects::operator=(const BufferObjects& a_obj) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::MESHES, a_obj.VAO); 

	if (a_obj.VAO != VAO) {
		refcnt_dec();
	} 
	VAO = a_obj.VAO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
	EBO = a_obj.EBO;

	return *this;
}

BufferObjects& BufferObjects::operator=(BufferObjects&& a_obj) noexcept {
	if (a_obj.VAO != VAO) {
		refcnt_dec();
	} 
	VAO = a_obj.VAO;
	EBO = a_obj.EBO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;

	a_obj.EBO = EMPTY_VBO;
	a_obj.VAO = EMPTY_VBO;
	a_obj.VBO_UVs = EMPTY_VBO;
	a_obj.VBO_pos = EMPTY_VBO;
	a_obj.VBO_normals = EMPTY_VBO;

	return *this;
}

BufferObjects::~BufferObjects() {
	refcnt_dec();
}

void BufferObjects::refcnt_dec() { 
	if (VAO != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::MESHES, VAO);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::MESHES, VAO)) {
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO_pos);
			glDeleteBuffers(1, &VBO_normals);
			glDeleteBuffers(1, &VBO_UVs);
			glDeleteBuffers(1, &EBO);
		}
	}
}

Texture::Texture(std::wstring a_path, TextureType a_type, int a_unit_id, bool a_flip_image, bool a_gamma_corr)
	:m_type{ a_type }, m_unit_id{ a_unit_id }, m_flipped{ a_flip_image }, m_gamma_corr{ a_gamma_corr }
{
	if (a_type != TextureType::DIFFUSE && a_type != TextureType::SPECULAR && a_type != TextureType::EMISSION)
		ERROR("[TEXTURE::TEXTURE] Bad texture type", Error_action::throwing);

	fs::path p = guess_path(a_path);
	if (p == fs::path())
		ERROR(std::format("[TEXTURE::TEXTURE] Bad texture path: {}", wstos(a_path)), Error_action::throwing);

	m_path = p.wstring();
	m_filename = p.filename().wstring();

	glGenTextures(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	int nrChannels{ };
	int width{ };
	int height{ };
	stbi_set_flip_vertically_on_load(a_flip_image);
	unsigned char* data = stbi_load(wstos(m_path).c_str(), &width, &height, &nrChannels, 0);

	unsigned in_format = GL_NONE;
	unsigned ex_format = GL_NONE;
	if (nrChannels == 1) {
		in_format = ex_format = GL_RED;
	}
	else if (nrChannels == 3) {
		auto cond = a_gamma_corr && (a_type == TextureType::DIFFUSE || a_type == TextureType::EMISSION);
		in_format = cond ? GL_SRGB : GL_RGB;
		ex_format = GL_RGB;
	}
	else if (nrChannels == 4) {
		auto cond = a_gamma_corr && (a_type == TextureType::DIFFUSE || a_type == TextureType::EMISSION);
		in_format = cond ? GL_SRGB_ALPHA : GL_RGBA;
		ex_format = GL_RGBA;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else {
		ERROR(std::format("[TEXTURE::TEXTURE] Unsupported texture format {} with {} number of channels.", wstos(m_path), nrChannels), Error_action::throwing);
	}

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, in_format, width, height, 0, ex_format, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	else {
		ERROR(std::format("[TEXTURE::TEXTURE] Couldn't load texture data at path {}", wstos(m_path)), Error_action::throwing);
	} 

	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::MIPMAP_LINEAR);
}

// Cubemaps
Texture::Texture(std::vector<std::wstring> a_paths, int texture_unit, bool a_flip_images, bool a_gamma_corr)
	:m_type{ TextureType::CUBEMAP }, m_unit_id{ texture_unit }, m_flipped{ a_flip_images }, m_gamma_corr{ a_gamma_corr }
{
	// Each cubemap face has to be a single texture
	if (a_paths.size() != 6)
		ERROR("[TEXTURE::TEXTURE] Wrong amount of textures to create a cubemap.", Error_action::throwing); 

	m_path = L"";
	for (const auto& path : a_paths) {
		fs::path p = guess_path(path);

		// Couldn't find texture path
		if (p == fs::path()) {
			m_path = L"";
			m_filename = L"";
			m_filenames.clear();
			ERROR(std::format("[TEXTURE::TEXTURE] Bad texture path: {}", wstos(path)), Error_action::throwing);
		}

		if (m_path == L"")
			m_path = p.parent_path().wstring();

		// Put your cubemap textures into same directory.  
		if (m_path != p.parent_path().wstring()) {
			m_path = L"";
			m_filename = L"";
			m_filenames.clear();
			ERROR("[TEXTURE::TEXTURE] Cubemap textures are not in the same directory. Move them to a single directory in order to create a cubemap.", Error_action::throwing);
		}

		m_filenames.push_back(p.filename().wstring());
	}

	glGenTextures(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	stbi_set_flip_vertically_on_load(a_flip_images);
	for (size_t i = 0; i < m_filenames.size(); ++i) {
		int nrChannels{};
		int width{};
		int height{};
		std::string full_path = (fs::path(m_path) / fs::path(m_filenames[i])).string();
		unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

		unsigned in_format = GL_NONE;
		unsigned ex_format = GL_NONE;
		if (nrChannels == 3) {
			in_format = a_gamma_corr ? GL_SRGB : GL_RGB;
			ex_format = GL_RGB;
		}
		else if (nrChannels == 4) {
			in_format = a_gamma_corr ? GL_SRGB_ALPHA : GL_RGBA;
			ex_format = GL_RGBA;

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else {
			ERROR(std::format("[TEXTURE::TEXTURE] Unsupported texture format {} with {} number of channels.", full_path, nrChannels), Error_action::throwing);
		}

		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, in_format, width, height, 0, ex_format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data); 
		}
		else {
			ERROR(std::format("[TEXTURE::TEXTURE] Couldn't load texture data at path {}", full_path), Error_action::throwing);
		}
	}

	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::LINEAR);
}

Texture::Texture(int a_width, int a_height, TextureType a_type) 
	:m_type{ a_type } 
{
	if (a_type != TextureType::COLOR_2D && a_type != TextureType::COLOR_3D && a_type != TextureType::CUBEMAP && a_type != TextureType::DEPTH && a_type != TextureType::DEPTH_STENCIL)
		ERROR("[TEXTURE::TEXTURE] Bad texture type", Error_action::throwing);

	glGenTextures(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id);
	if (a_type == TextureType::COLOR_3D || a_type == TextureType::CUBEMAP) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_id); 
	}
	else {
		glBindTexture(GL_TEXTURE_2D, m_id); 
	}

	switch (a_type) {
	case TextureType::COLOR_2D:      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, a_width, a_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); break;
	case TextureType::DEPTH:         glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, a_width, a_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); break;
	case TextureType::DEPTH_STENCIL: glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, a_width, a_height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL); break;
	case TextureType::CUBEMAP:
	case TextureType::COLOR_3D:
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, a_width, a_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); 
		}
		break;
	}

	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::LINEAR);
}

Texture::Texture(int a_width, int a_height, int a_samples, TextureType a_type)
	:m_samples{ a_samples }, m_type{ a_type }
{
	if (a_type != TextureType::COLOR_2D && a_type != TextureType::DEPTH && a_type != TextureType::DEPTH_STENCIL)
		ERROR("[TEXTURE::TEXTURE] Bad MSAA texture type.", Error_action::throwing);

	glGenTextures(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id); 
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id); 

	switch (a_type) {
	case TextureType::COLOR_2D:      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, a_samples, GL_RGB, a_width, a_height, GL_TRUE); break;
	case TextureType::DEPTH:         glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, a_samples, GL_DEPTH_COMPONENT, a_width, a_height, GL_TRUE); break;
	case TextureType::DEPTH_STENCIL: glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, a_samples, GL_DEPTH24_STENCIL8, a_width, a_height, GL_TRUE); break;
	}

	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::LINEAR);
}

Texture::Texture(const Texture& a_texture) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, a_texture.m_id);

	m_id = a_texture.m_id;
	m_path = a_texture.m_path;
	m_type = a_texture.m_type;
	m_wrap = a_texture.m_wrap;
	m_filter = a_texture.m_filter;
	m_comp = a_texture.m_comp;
	m_unit_id = a_texture.m_unit_id;
	m_samples = a_texture.m_samples;
	m_filename = a_texture.m_filename;
	m_filenames = a_texture.m_filenames;
	m_flipped = a_texture.m_flipped;
	m_gamma_corr = a_texture.m_gamma_corr;
}

// When moving an object, reference count shouldn't increment.
Texture::Texture(Texture&& a_texture) noexcept {
	m_id = a_texture.m_id;
	m_path = std::move(a_texture.m_path);
	m_type = a_texture.m_type;
	m_wrap = a_texture.m_wrap;
	m_filter = a_texture.m_filter;
	m_comp = a_texture.m_comp;
	m_unit_id = a_texture.m_unit_id;
	m_samples = a_texture.m_samples;
	m_filename = std::move(a_texture.m_filename);
	m_filenames = std::move(a_texture.m_filenames);
	m_flipped = a_texture.m_flipped;
	m_gamma_corr = a_texture.m_gamma_corr;

	a_texture.m_id = EMPTY_VBO;
}

Texture& Texture::operator=(const Texture& a_texture) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, a_texture.m_id);

	if (m_id != a_texture.m_id) {
		refcnt_dec();
	}
	m_id = a_texture.m_id;
	m_path = a_texture.m_path;
	m_type = a_texture.m_type;
	m_wrap = a_texture.m_wrap;
	m_filter = a_texture.m_filter;
	m_comp = a_texture.m_comp;
	m_unit_id = a_texture.m_unit_id;
	m_samples = a_texture.m_samples;
	m_filename = a_texture.m_filename;
	m_filenames = a_texture.m_filenames;
	m_flipped = a_texture.m_flipped;
	m_gamma_corr = a_texture.m_gamma_corr;

	return *this;
}

// When moving an object, reference count shouldn't increment.
Texture& Texture::operator=(Texture&& a_texture) noexcept {
	if (m_id != a_texture.m_id) {
		refcnt_dec();
	}
	m_id = a_texture.m_id;
	m_path = std::move(a_texture.m_path);
	m_type = a_texture.m_type;
	m_wrap = a_texture.m_wrap;
	m_filter = a_texture.m_filter;
	m_comp = a_texture.m_comp;
	m_unit_id = a_texture.m_unit_id;
	m_samples = a_texture.m_samples;
	m_filename = std::move(a_texture.m_filename);
	m_filenames = std::move(a_texture.m_filenames);
	m_flipped = a_texture.m_flipped;
	m_gamma_corr = a_texture.m_gamma_corr;

	a_texture.m_id = EMPTY_VBO;

	return *this;
}

Texture::~Texture() {
	refcnt_dec();
}

void Texture::refcnt_dec() {
	if (m_id != EMPTY_VBO && m_type != TextureType::NONE) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::TEXTURES, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::TEXTURES, m_id)) {
			std::cout << "DELETING: ID: " << m_id << ", PATH: [" << wstos(m_path) << "]" << std::endl;
			glDeleteTextures(1, &m_id);
		}
	} 
}

void Texture::set_border_color(const glm::vec3& a_border_color) noexcept {
	float arr_border_color[] = { a_border_color.x, a_border_color.y, a_border_color.z, 1.f };
	GLuint tex_type = (m_type == TextureType::COLOR_3D || m_type == TextureType::CUBEMAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D; 
	glBindTexture(tex_type, m_id); 
	glTexParameterfv(tex_type, GL_TEXTURE_BORDER_COLOR, arr_border_color);
}

void Texture::set_unit_id(int a_unit_id) noexcept {
	m_unit_id = a_unit_id;
}

void Texture::set_type(TextureType a_type) noexcept {
	m_type = a_type;
}

void Texture::set_wrap(TextureWrap a_wrap) noexcept {
	m_wrap = a_wrap;

	GLuint tex_type = 0;
	GLuint tex_wrap = 0; 

	// Error(1280): Multisample texture targets don't support sampler state.
	if (m_samples > 1) return;
	else if (m_type == TextureType::COLOR_3D || m_type == TextureType::CUBEMAP) {
		tex_type = GL_TEXTURE_CUBE_MAP;
	}
	else {
		tex_type = GL_TEXTURE_2D;
	}

	switch (m_wrap) {
	case TextureWrap::REPEAT: tex_wrap = GL_REPEAT; break;
	case TextureWrap::CLAMP_EDGE: tex_wrap = GL_CLAMP_TO_EDGE; break;
	case TextureWrap::CLAMP_BORDER: tex_wrap = GL_CLAMP_TO_BORDER; break;
	case TextureWrap::MIRROR_REPEAT: tex_wrap = GL_MIRRORED_REPEAT; break;
	default: ERROR("[TEXTURE::SET_WRAP] Wrong wrap type.", Error_action::throwing);
	}

	glBindTexture(tex_type, m_id); 
	glTexParameterf(tex_type, GL_TEXTURE_WRAP_S, tex_wrap);
	glTexParameterf(tex_type, GL_TEXTURE_WRAP_T, tex_wrap);
	if (tex_type == GL_TEXTURE_CUBE_MAP) { 
		glTexParameterf(tex_type, GL_TEXTURE_WRAP_R, tex_wrap); 
	}
}

void Texture::set_filter(TextureFilter a_filter) noexcept {
	m_filter = a_filter;

	GLuint tex_min = 0;
	GLuint tex_mag = 0;
	GLuint tex_type = 0;

	// Error(1280): Multisample texture targets don't support sampler state.
	if (m_samples > 1) return;
	else if (m_type == TextureType::COLOR_3D || m_type == TextureType::CUBEMAP) {
		tex_type = GL_TEXTURE_CUBE_MAP;
	}
	else {
		tex_type = GL_TEXTURE_2D;
	}

	switch (a_filter) {
	case TextureFilter::LINEAR: 
		tex_min = GL_LINEAR;
		tex_mag = GL_LINEAR; 
		break;
	case TextureFilter::NEAREST: 
		tex_min = GL_NEAREST;
		tex_mag = GL_NEAREST; 
		break;
	case TextureFilter::MIPMAP_LINEAR: 
		tex_min = GL_LINEAR_MIPMAP_LINEAR; 
		tex_mag = GL_LINEAR; 
		break;
	case TextureFilter::MIPMAP_NEAREST: 
		tex_min = GL_NEAREST_MIPMAP_NEAREST; 
		tex_mag = GL_NEAREST; 
		break;
	default: ERROR("[TEXTURE::SET_FILTER] Wrong filter type.", Error_action::throwing);
	}

	glBindTexture(tex_type, m_id); 
	glTexParameteri(tex_type, GL_TEXTURE_MIN_FILTER, tex_min);
	glTexParameteri(tex_type, GL_TEXTURE_MAG_FILTER, tex_mag);
	if (a_filter == TextureFilter::MIPMAP_LINEAR || a_filter == TextureFilter::MIPMAP_NEAREST) {
		glGenerateMipmap(tex_type);
	}
}

void Texture::set_comp_func(TextureCompFunc a_comp_func) noexcept { 
	m_comp = a_comp_func;

	GLuint tex_type = 0;
	GLuint tex_comp = 0; 

	// TODO: Might be wrong. Check. Error(1280): Multisample texture targets don't support sampler state.
	if (m_samples > 1) return;
	else if (m_type == TextureType::COLOR_3D || m_type == TextureType::CUBEMAP) {
		tex_type = GL_TEXTURE_CUBE_MAP;
	}
	else {
		tex_type = GL_TEXTURE_2D;
	}

	switch (a_comp_func) {
	case TextureCompFunc::LESS:     tex_comp = GL_LESS; break;
	case TextureCompFunc::EQUAL:    tex_comp = GL_EQUAL; break;
	case TextureCompFunc::NEVER:    tex_comp = GL_NEVER; break;
	case TextureCompFunc::ALWAYS:   tex_comp = GL_ALWAYS; break;
	case TextureCompFunc::LEQUAL:   tex_comp = GL_LEQUAL; break;
	case TextureCompFunc::GEQUAL:   tex_comp = GL_GEQUAL; break;
	case TextureCompFunc::GREATER:  tex_comp = GL_GREATER; break;
	case TextureCompFunc::NOTEQUAL: tex_comp = GL_NOTEQUAL; break;
	case TextureCompFunc::NONE:
		glBindTexture(tex_type, m_id);
		glTexParameteri(tex_type, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		return;
	default: ERROR("[TEXTURE::SET_COMP_FUNC] Wrong comparison function type.", Error_action::throwing);
	}

	glBindTexture(tex_type, m_id);
	glTexParameteri(tex_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(tex_type, GL_TEXTURE_COMPARE_FUNC, tex_comp);
}

void Texture::activate() const noexcept {
	if (m_type != TextureType::NONE) {
		glActiveTexture(GL_TEXTURE0 + m_unit_id);
		if (m_samples > 1) {
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_id);
		}
		else if (m_type == TextureType::CUBEMAP || m_type == TextureType::COLOR_3D) { 
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_id); 
		}
		else {
			glBindTexture(GL_TEXTURE_2D, m_id); 
		}
	}
}

TextureType Texture::get_type() const noexcept {
	return m_type;
}

TextureWrap Texture::get_wrap() const noexcept {
	return m_wrap;
}

TextureCompFunc Texture::get_comp_func() const noexcept {
	return m_comp;
}

TextureFilter Texture::get_filter() const noexcept {
	return m_filter;
}

std::wstring Texture::get_path() const noexcept {
	return m_path;
}

std::wstring Texture::get_filename() const noexcept {
	return m_filename;
}

std::vector<std::wstring> Texture::get_filenames() const noexcept {
	return m_filenames;
}

GLuint Texture::get_id() const noexcept {
	return m_id;
}

int Texture::get_unit_id() const noexcept {
	return m_unit_id;
}

bool Texture::is_flipped() const noexcept {
	return m_flipped;
}

bool Texture::is_gamma_corr() const noexcept {
	return m_gamma_corr;
}

RenderBuffer::RenderBuffer(int a_width, int a_height, RenderBufferType a_type) :m_type{ a_type } {
	if (a_type != RenderBufferType::COLOR && a_type != RenderBufferType::DEPTH && a_type != RenderBufferType::DEPTH_STENCIL)
		ERROR("[RENDERBUFFER::RENDERBUFFER] Bad renderbuffer type.", Error_action::throwing);

	glGenRenderbuffers(1, &m_rbo);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);

	switch (a_type) {
	case RenderBufferType::COLOR:         glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, a_width, a_height); break;
	case RenderBufferType::DEPTH:         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, a_width, a_height); break;
	case RenderBufferType::DEPTH_STENCIL: glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, a_width, a_height); break;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::RenderBuffer(int a_width, int a_height, int a_samples, RenderBufferType a_type) 
	:m_samples{ a_samples }, m_type{ a_type }
{ 
	if (a_type != RenderBufferType::COLOR && a_type != RenderBufferType::DEPTH && a_type != RenderBufferType::DEPTH_STENCIL)
		ERROR("[RENDERBUFFER::RENDERBUFFER] Bad MSAA renderbuffer type.", Error_action::throwing);

	glGenRenderbuffers(1, &m_rbo);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo); 

	switch (a_type) {
	case RenderBufferType::COLOR:         glRenderbufferStorageMultisample(GL_RENDERBUFFER, a_samples, GL_RGB, a_width, a_height); break;
	case RenderBufferType::DEPTH:         glRenderbufferStorageMultisample(GL_RENDERBUFFER, a_samples, GL_DEPTH_COMPONENT, a_width, a_height); break;
	case RenderBufferType::DEPTH_STENCIL: glRenderbufferStorageMultisample(GL_RENDERBUFFER, a_samples, GL_DEPTH24_STENCIL8, a_width, a_height); break;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::RenderBuffer(const RenderBuffer& a_ren_buf) {
	Application::get_instance().get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, a_ren_buf.get_id());

	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;
}

RenderBuffer::RenderBuffer(RenderBuffer&& a_ren_buf) noexcept {
	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;

	a_ren_buf.m_rbo = EMPTY_VBO;
	a_ren_buf.m_type = RenderBufferType::NONE;
}

RenderBuffer& RenderBuffer::operator=(const RenderBuffer& a_ren_buf) {
	Application::get_instance().get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, a_ren_buf.get_id());

	if (m_rbo != a_ren_buf.m_rbo) {
		refcnt_dec();
	}
	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;

	return *this;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& a_ren_buf) noexcept {
	if (m_rbo != a_ren_buf.m_rbo) {
		refcnt_dec();
	}
	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;

	a_ren_buf.m_rbo = EMPTY_VBO;
	a_ren_buf.m_type = RenderBufferType::NONE;

	return *this;
}

RenderBuffer::~RenderBuffer() {
	refcnt_dec();
}

void RenderBuffer::refcnt_dec() {
	if (m_rbo != EMPTY_VBO && m_type != RenderBufferType::NONE) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::RENDER_BUFFERS, m_rbo);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::RENDER_BUFFERS, m_rbo)) {
			glDeleteRenderbuffers(1, &m_rbo);
		}
	} 
}

GLuint RenderBuffer::get_id() const noexcept {
	return m_rbo;
}

RenderBufferType RenderBuffer::get_type() const noexcept {
	return m_type;
}

AttachmentBuffer::AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples) 
	:m_type{ a_attach_type }, m_buf_type{ a_buf_type }
{
	if (a_buf_type == AttachmentBufferType::TEXTURE) {
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      m_attachment = Texture(a_width, a_height, TextureType::COLOR_2D); break;
		case AttachmentType::COLOR_3D:      m_attachment = Texture(a_width, a_height, TextureType::COLOR_3D); break;
		case AttachmentType::DEPTH:         m_attachment = Texture(a_width, a_height, TextureType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = Texture(a_width, a_height, TextureType::DEPTH_STENCIL); break;
		}
	}
	else if (a_buf_type == AttachmentBufferType::MSAA_TEXTURE) {
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      m_attachment = Texture(a_width, a_height, a_samples, TextureType::COLOR_2D); break;
		case AttachmentType::COLOR_3D:      m_attachment = Texture(a_width, a_height, a_samples, TextureType::COLOR_3D); break;
		case AttachmentType::DEPTH:         m_attachment = Texture(a_width, a_height, a_samples, TextureType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = Texture(a_width, a_height, a_samples, TextureType::DEPTH_STENCIL); break;
		}
	}
	else if (a_buf_type == AttachmentBufferType::MSAA_RENDER_BUFFER) {
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      m_attachment = RenderBuffer(a_width, a_height, a_samples, RenderBufferType::COLOR); break;
		case AttachmentType::DEPTH:         m_attachment = RenderBuffer(a_width, a_height, a_samples, RenderBufferType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = RenderBuffer(a_width, a_height, a_samples, RenderBufferType::DEPTH_STENCIL); break;
		case AttachmentType::COLOR_3D:
			ERROR("[ATTACHMENTBUFFER::ATTACHMENTBUFFER] Please use attachment type COLOR_2D for render buffers.", Error_action::throwing);
			break;
		}
	}
	else if (a_buf_type == AttachmentBufferType::RENDER_BUFFER) {
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      m_attachment = RenderBuffer(a_width, a_height, RenderBufferType::COLOR); break;
		case AttachmentType::DEPTH:         m_attachment = RenderBuffer(a_width, a_height, RenderBufferType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = RenderBuffer(a_width, a_height, RenderBufferType::DEPTH_STENCIL); break;
		case AttachmentType::COLOR_3D:
			ERROR("[ATTACHMENTBUFFER::ATTACHMENTBUFFER] Please use attachment type COLOR_2D for render buffers.", Error_action::throwing);
			break;
		}
	}
	else if (a_buf_type == AttachmentBufferType::NONE) {
		return;
	}
	else {
		ERROR("[ATTACHMENTBUFFER::ATTACHMENTBUFFER] Bad attachment buffer type.", Error_action::throwing);
	}
}

void AttachmentBuffer::activate() const noexcept {
	if (std::holds_alternative<Texture>(m_attachment)) {
		std::get<Texture>(m_attachment).activate();
	}
	else if (std::holds_alternative<RenderBuffer>(m_attachment)) {
		// WARNING: Having color renderbuffer is plain weird but possible. Not sure how 
		// to handle it.
	}
}

AttachmentType AttachmentBuffer::get_type() const noexcept {
	return m_type;
}

AttachmentBufferType AttachmentBuffer::get_buf_type() const noexcept {
	return m_buf_type;
}

std::variant<Texture, RenderBuffer>& AttachmentBuffer::get_attachment() noexcept {
	return m_attachment;
}

FrameBuffer::FrameBuffer(int a_width, int a_height) :m_width{ a_width }, m_height{ a_height } {
	glGenFramebuffers(1, &m_fbo);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
}

FrameBuffer::FrameBuffer(FrameBuffer&& a_frame_buf) noexcept {
	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_samples = a_frame_buf.m_samples;
	m_color_attachment = std::move(a_frame_buf.m_color_attachment);
	m_depth_attachment = std::move(a_frame_buf.m_depth_attachment);
	m_depth_stencil_attachment = std::move(a_frame_buf.m_depth_stencil_attachment);

	a_frame_buf.m_fbo = EMPTY_VBO;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& a_frame_buf) noexcept {
	if (m_fbo != a_frame_buf.m_fbo) {
		refcnt_dec();
	}
	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_samples = a_frame_buf.m_samples;
	m_color_attachment = std::move(a_frame_buf.m_color_attachment);
	m_depth_attachment = std::move(a_frame_buf.m_depth_attachment);
	m_depth_stencil_attachment = std::move(a_frame_buf.m_depth_stencil_attachment);

	a_frame_buf.m_fbo = EMPTY_VBO;

	return *this;
}

FrameBuffer::~FrameBuffer() {
	refcnt_dec();
}

void FrameBuffer::refcnt_dec() {
	if (m_fbo != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::FRAME_BUFFERS, m_fbo)) {
			glDeleteFramebuffers(1, &m_fbo);
		}
	} 
}

void FrameBuffer::attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples) {
	if (a_attach_type == AttachmentType::NONE || a_samples <= 0 ||
		((a_buf_type == AttachmentBufferType::TEXTURE || a_buf_type == AttachmentBufferType::RENDER_BUFFER) && a_samples > 1)) {
		return; 
	} 

	if (m_fbo == EMPTY_VBO) {
		glGenFramebuffers(1, &m_fbo);
		Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
	}

	m_samples = a_samples; 
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); 
	AttachmentBuffer new_attachment(m_width, m_height, a_attach_type, a_buf_type, m_samples); 
	if (a_attach_type == AttachmentType::COLOR_2D || a_attach_type == AttachmentType::COLOR_3D) {
		m_color_attachment = new_attachment;
	}
	else if (a_attach_type == AttachmentType::DEPTH_STENCIL) {
		m_depth_stencil_attachment = new_attachment;
	}
	else if (a_attach_type == AttachmentType::DEPTH) {
		m_depth_attachment = new_attachment;
	}

	if (a_buf_type == AttachmentBufferType::TEXTURE) {
		Texture& tex = std::get<Texture>(new_attachment.get_attachment());
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.get_id(), 0); break;
		case AttachmentType::COLOR_3D:      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex.get_id(), 0); break;
		case AttachmentType::DEPTH:         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex.get_id(), 0); break;
		case AttachmentType::DEPTH_STENCIL: glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex.get_id(), 0); break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] Wrong attachment type.", Error_action::throwing);
		}
	}
	else if (a_buf_type == AttachmentBufferType::MSAA_TEXTURE) {
		Texture& tex = std::get<Texture>(new_attachment.get_attachment());
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex.get_id(), 0); break;
		case AttachmentType::DEPTH:         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, tex.get_id(), 0); break;
		case AttachmentType::DEPTH_STENCIL: glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, tex.get_id(), 0); break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] Wrong attachment type.", Error_action::throwing);
		} 
	}
	else if (a_buf_type == AttachmentBufferType::RENDER_BUFFER || a_buf_type == AttachmentBufferType::MSAA_RENDER_BUFFER) {
		RenderBuffer& ren_buf = std::get<RenderBuffer>(new_attachment.get_attachment());
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ren_buf.get_id()); break;
		case AttachmentType::DEPTH:         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ren_buf.get_id()); break;
		case AttachmentType::DEPTH_STENCIL: glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ren_buf.get_id()); break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] Wrong attachment type.", Error_action::throwing);
		}
	}
	else if (a_buf_type == AttachmentBufferType::NONE) {
		switch (a_attach_type) {
		case AttachmentType::COLOR_2D:
		case AttachmentType::COLOR_3D:
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] None depth/stencil buffers not handled.", Error_action::throwing);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::attach_cubemap_face(GLenum a_cubemap_face) { 
	if (m_fbo == EMPTY_VBO) {
		glGenFramebuffers(1, &m_fbo);
		Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Ensure that attached color buffer is a cubemap
	if (!std::holds_alternative<Texture>(m_color_attachment.get_attachment()) || m_color_attachment.get_type() != AttachmentType::COLOR_3D) {
		attach(AttachmentType::COLOR_3D, AttachmentBufferType::TEXTURE); 
	}

	Texture& attached_color_cubemap = std::get<Texture>(m_color_attachment.get_attachment());

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, a_cubemap_face, attached_color_cubemap.get_id(), 0); 
	glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

AttachmentBuffer& FrameBuffer::get_color_attachment_buffer() noexcept {
	return m_color_attachment;
}

AttachmentBuffer& FrameBuffer::get_depth_attachment_buffer() noexcept {
	return m_depth_attachment;
}

AttachmentBuffer& FrameBuffer::get_depth_stencil_attachment_buffer() noexcept {
	return m_depth_stencil_attachment;
}

void FrameBuffer::activate_color() const noexcept {
	m_color_attachment.activate(); 
}

void FrameBuffer::activate_depth() const noexcept {
	m_depth_attachment.activate();
}

GLuint FrameBuffer::get_id() const noexcept {
	return m_fbo;
}

void FrameBuffer::bind() const noexcept {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind() const noexcept {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::check_status() const noexcept {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER); 
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return true;
}

void FrameBuffer::set_resolution(int a_width, int a_height) {
	if (m_width == a_width && m_height == a_height) return;

	m_width = a_width;
	m_height = a_height;

	if (auto att_type = m_color_attachment.get_type(); att_type != AttachmentType::NONE) {
		auto buf_type = m_color_attachment.get_buf_type();
		attach(att_type, buf_type, m_samples);
	}
	if (auto att_type = m_depth_attachment.get_type(); att_type != AttachmentType::NONE) {
		auto buf_type = m_depth_attachment.get_buf_type();
		attach(att_type, buf_type, m_samples);
	}
	if (auto att_type = m_depth_stencil_attachment.get_type(); att_type != AttachmentType::NONE) {
		auto buf_type = m_depth_stencil_attachment.get_buf_type();
		attach(att_type, buf_type, m_samples);
	}
}

bool FrameBuffer::set_samples(int a_samples) {
	if (!check_status() || a_samples == m_samples) {
		return false;
	}

	auto convert_buffer_type = [](const AttachmentBufferType& buf_type) {
		switch (buf_type) {
		case AttachmentBufferType::MSAA_TEXTURE:       return AttachmentBufferType::TEXTURE;
		case AttachmentBufferType::MSAA_RENDER_BUFFER: return AttachmentBufferType::RENDER_BUFFER;
		case AttachmentBufferType::TEXTURE:			   return AttachmentBufferType::MSAA_TEXTURE;
		case AttachmentBufferType::RENDER_BUFFER:	   return AttachmentBufferType::MSAA_RENDER_BUFFER; 
		}
		return AttachmentBufferType::NONE;
	};

	if (a_samples == 1 && m_samples > 1 || a_samples > 1 && m_samples == 1) { 
		m_samples = a_samples;
		attach(m_color_attachment.get_type(), convert_buffer_type(m_color_attachment.get_buf_type()), a_samples);
		attach(m_depth_attachment.get_type(), convert_buffer_type(m_depth_attachment.get_buf_type()), a_samples);
		attach(m_depth_stencil_attachment.get_type(), convert_buffer_type(m_depth_stencil_attachment.get_buf_type()), a_samples);
	}
	else {
		m_samples = a_samples;
		attach(m_color_attachment.get_type(), m_color_attachment.get_buf_type(), a_samples);
		attach(m_depth_attachment.get_type(), m_depth_attachment.get_buf_type(), a_samples);
		attach(m_depth_stencil_attachment.get_type(), m_depth_stencil_attachment.get_buf_type(), a_samples); 
	}
	return true;
}

int FrameBuffer::get_width() const noexcept {
	return m_width;
}

int FrameBuffer::get_height() const noexcept {
	return m_height;
}

int FrameBuffer::get_samples() const noexcept {
	return m_samples;
}

UniformBuffer::UniformBuffer(const UniformBuffer& a_uni_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, a_uni_buf.m_id);

	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point; 
}

UniformBuffer::UniformBuffer(UniformBuffer&& a_uni_buf) noexcept {
	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = std::move(a_uni_buf.m_elements);
	m_binding_point = a_uni_buf.m_binding_point;

	a_uni_buf.m_id = EMPTY_VBO; 
}

UniformBuffer::~UniformBuffer() {
	refcnt_dec();
}

void UniformBuffer::refcnt_dec() {
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::UNIFORM_BUFFERS, m_id)) {
			glDeleteFramebuffers(1, &m_id);
		}
	} 
}

void UniformBuffer::refcnt_inc() {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
}

UniformBuffer& UniformBuffer::operator=(const UniformBuffer& a_uni_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, a_uni_buf.m_id);

	if (m_id != a_uni_buf.m_id) { 
		refcnt_dec();
	}
	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point;

	return *this; 
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& a_uni_buf) noexcept {
	if (m_id != a_uni_buf.m_id) { 
		refcnt_dec();
	}
	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = std::move(a_uni_buf.m_elements);
	m_binding_point = a_uni_buf.m_binding_point;

	a_uni_buf.m_id = EMPTY_VBO;

	return *this; 
}

UniformBufferElement& UniformBuffer::operator[](const std::string& a_uniform_name) {
	auto it = m_elements.find(a_uniform_name);
	if (it == m_elements.end())
		ERROR(std::format("[UNIFORMBUFFER::OPERATOR[]] Uniform name \"{}\" is not present in this uniform buffer object.", a_uniform_name), Error_action::throwing);
	return (*it).second; 
}

bool UniformBuffer::check_status() const noexcept {
	if (m_id == EMPTY_VBO || m_binding_point == -1)
		return false;
	return true; 
}

void UniformBuffer::create_buffer() noexcept {
	if (m_id == EMPTY_VBO) {
		glGenBuffers(1, &m_id);
		Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_id);
	glBufferData(GL_UNIFORM_BUFFER, m_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); 
}

void UniformBuffer::set_binding_point(int a_binding_point) noexcept {
	m_binding_point = a_binding_point;
	glBindBufferBase(GL_UNIFORM_BUFFER, a_binding_point, m_id); 
}

void UniformBuffer::clear() {
	m_elements.clear();
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::UNIFORM_BUFFERS, m_id)) {
			glDeleteFramebuffers(1, &m_id);
		}
		m_id = EMPTY_VBO;
	} 
}

UniformBuffer::NameUniMap UniformBuffer::get_elements() const noexcept {
	return m_elements; 
} 
}
