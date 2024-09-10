#pragma once

#include <glad/glad.h>
#include <variant>
#include <vector>
#include <memory>
#include <filesystem>

#include "chill_engine/assert.hpp"

constexpr int EMPTY_VBO = 0;
constexpr int ATTRIB_POS_LOCATION = 0;
constexpr int ATTRIB_TEX_LOCATION = 1;
constexpr int ATTRIB_NORMAL_LOCATION = 2;
constexpr int ATTRIB_COLOR_LOCATION = 3;

namespace chill_engine { 
namespace fs = std::filesystem;

fs::path guess_path(const std::wstring& a_path);

enum class RenderBufferType {
	COLOR,
	DEPTH,
	DEPTH_STENCIL,
	NONE
};

enum class AttachmentType {
	COLOR_2D,
	COLOR_3D,
	DEPTH,
	DEPTH_STENCIL,
	NONE,
};

enum class AttachmentBufferType {
	TEXTURE,
	RENDER_BUFFER,
};

enum class TextureType {
	// Textures for MaterialMap
	DIFFUSE,
	SPECULAR,
	EMISSION,

	// Textures for AttachmentBuffer
	COLOR_2D,
	COLOR_3D,
	DEPTH,
	DEPTH_STENCIL,

	CUBEMAP,

	NONE,
};
std::ostream& operator<<(std::ostream& os, const TextureType& a_type);

struct BufferObjects {
	BufferObjects() = default;
	BufferObjects(const BufferObjects& a_obj);
	BufferObjects(BufferObjects&& a_obj) noexcept;
	~BufferObjects();

	auto operator=(BufferObjects& a_obj) -> BufferObjects&;
	auto operator=(BufferObjects&& a_obj) noexcept -> BufferObjects&;

	GLuint VAO = EMPTY_VBO;
	GLuint VBO_UVs = EMPTY_VBO;
	GLuint VBO_pos = EMPTY_VBO;
	GLuint VBO_normals = EMPTY_VBO;
	GLuint EBO = EMPTY_VBO;
};

class Texture {
public:
	Texture() = default;
	Texture(std::wstring a_path, TextureType a_type, bool a_flip_image, int texture_unit);
	Texture(std::vector<std::wstring> a_paths, bool a_flip_images, int texture_unit);
	Texture(int a_width, int a_height, TextureType a_type);
	Texture(const Texture& a_texture);
	Texture(Texture&& a_texture) noexcept;
	~Texture();

	auto operator=(const Texture& a_texture) -> Texture&;
	auto operator=(Texture&& a_texture) noexcept -> Texture&;

	auto set_unit_id(int a_unit_id) -> void;
	auto set_type(TextureType a_type) -> void;
	auto activate() const -> void;

	auto get_id() const -> GLuint;
	auto get_path() const -> std::wstring;
	auto get_filename() const -> std::wstring;
	auto get_filenames() const -> std::vector<std::wstring>;
	auto get_type() const -> TextureType;
	auto get_unit_id() const -> int;

	auto is_flipped() const -> bool;

private:
	GLuint m_id = EMPTY_VBO;
	std::wstring m_path = L"";
	std::wstring m_filename = L"";
	std::vector<std::wstring> m_filenames = {};
	TextureType m_type = TextureType::NONE;
	int m_unit_id = 0;
	bool m_flipped = false;
};

class RenderBuffer {
public:
	RenderBuffer() = default;
	RenderBuffer(int a_width, int a_height, RenderBufferType a_type);
	RenderBuffer(const RenderBuffer& a_ren_buf);
	RenderBuffer(RenderBuffer&& a_ren_buf) noexcept;
	~RenderBuffer();

	auto operator=(const RenderBuffer& a_ren_buf) -> RenderBuffer&;
	auto operator=(RenderBuffer&& a_ren_buf) noexcept -> RenderBuffer&;

	auto get_id() const -> GLuint;
	auto get_type() const -> RenderBufferType;

private:
	GLuint m_rbo = EMPTY_VBO;
	RenderBufferType m_type = RenderBufferType::NONE;
};

class AttachmentBuffer {
public:
	AttachmentBuffer() = default;
	AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type);

	auto activate() const -> void;
	auto get_type() const -> AttachmentType;
	auto get_attachment() const -> std::variant<Texture, RenderBuffer>;

private:
	std::variant<Texture, RenderBuffer> m_attachment;
	AttachmentType m_type = AttachmentType::NONE;
};

class Framebuffer {
public:
	Framebuffer() = default;
	Framebuffer(int a_width, int a_height);
	Framebuffer(const Framebuffer& a_frame_buf);
	Framebuffer(Framebuffer&& a_frame_buf) noexcept;
	~Framebuffer();

	auto operator=(const Framebuffer& a_frame_buf) -> Framebuffer&;
	auto operator=(Framebuffer&& a_frame_buf) noexcept -> Framebuffer&;

	auto attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type) -> void;
	auto attach_cubemap_face(GLenum a_cubemap_face) -> void;
	auto get_attachment_buffer(AttachmentType a_type) const -> AttachmentBuffer;
	auto activate_color() const -> void;
	auto get_id() const -> GLuint;
	auto bind() const -> void;
	auto unbind() const -> void;
	auto check_status() -> bool;
	auto set_width(int a_width) -> void;
	auto set_height(int a_height) -> void;
	auto get_width() const -> int;
	auto get_height() const -> int;

private:
	GLuint m_fbo = EMPTY_VBO;
	int m_width = 0;
	int m_height = 0;
	AttachmentBuffer m_color_attachment{};
	AttachmentBuffer m_depth_attachment{};
	AttachmentBuffer m_depth_stencil_attachment{};
};
}