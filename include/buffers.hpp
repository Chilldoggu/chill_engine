#pragma once

#include <glad/glad.h>

#include <variant>
#include <vector>
#include <memory>
#include <filesystem>

#include "assert.hpp"

constexpr int EMPTY_VBO = 0;
constexpr int ATTRIB_POS_LOCATION    = 0;
constexpr int ATTRIB_TEX_LOCATION    = 1;
constexpr int ATTRIB_NORMAL_LOCATION = 2;
constexpr int ATTRIB_COLOR_LOCATION  = 3; 

std::filesystem::path get_proj_path();
 
enum class RenderBufferType {
	COLOR,
	DEPTH,
	DEPTH_STENCIL, 
	NONE
};

enum class AttachmentType {
	COLOR,
	DEPTH,
	DEPTH_STENCIL,
	NONE,
};

enum class AttachmentBufferType {
	TEXTURE,
	RENDER_BUFFER,
};

enum class TextureType {
	DIFFUSE,
	SPECULAR,
	EMISSION,

	COLOR,
	DEPTH,
	DEPTH_STENCIL,

    NONE,
}; 
std::ostream& operator<<(std::ostream& os, const TextureType& a_type);

class Texture {
public:
	Texture() = default;
	// Load texture from filesystem
    Texture(std::wstring a_dir, TextureType a_type, int texture_unit);
	// Create empty texture buffer. Used for frame buffer attachment.
    Texture(int a_width, int a_height, TextureType a_type);
    ~Texture();

	auto clear() -> void;
    auto load_texture(std::wstring a_name, TextureType a_type, int a_texture_unit, bool a_flip_UVs = true) -> void;
	auto create_texture(int a_width, int a_height, TextureType a_type) -> void;
	auto set_texture_unit(int a_unit_id) -> void;
	auto set_texture_type(TextureType a_type) -> void;
    auto activate() const -> void; 

    auto get_id() const -> unsigned int;
    auto get_dir() const -> std::wstring;
    auto get_type() const -> TextureType;
	auto get_unit_id() const -> int;

private:
    int m_texture_unit = 0;
    std::wstring m_dir = L"";
    unsigned int m_texture_id = EMPTY_VBO;
    TextureType m_type = TextureType::NONE;
};

class RenderBuffer {
public:
	RenderBuffer() = default; 
	RenderBuffer(int a_width, int a_height, RenderBufferType a_type); 
	~RenderBuffer();

	auto get_id() const -> unsigned int; 
	auto get_type() const -> RenderBufferType;

private:
	unsigned int m_rbo = EMPTY_VBO;
	RenderBufferType m_type = RenderBufferType::NONE;
};

class AttachmentBuffer {
public:
	AttachmentBuffer() = default;
	AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type);

	auto activate() const -> void; 
	auto get_type() const -> AttachmentType; 
	auto get_attachment() const -> std::variant<std::shared_ptr<Texture>, std::shared_ptr<RenderBuffer>>;
private:
	// Shared pointers are used because Texture and RenderBuffer classes have annoying destructors that I don't want to call.
	std::variant<std::shared_ptr<Texture>, std::shared_ptr<RenderBuffer>> m_attachment;
	AttachmentType m_type = AttachmentType::NONE;
};

class Framebuffer { 
public:
	Framebuffer(int a_width, int a_height); 
	~Framebuffer();

	auto attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type) -> void;
	auto get_attachment_buffer(AttachmentType a_type) const -> AttachmentBuffer;
	auto activate_color() const -> void; 
	auto get_id() const -> unsigned int; 
	auto bind() -> void; 
	auto unbind() -> void; 
	auto check_status() -> bool; 
	auto set_width(int a_width) -> void; 
	auto set_height(int a_height) -> void; 
	auto get_width() const -> int; 
	auto get_height() const -> int;

private:
	unsigned int m_fbo = EMPTY_VBO;
	int m_width;
	int m_height;
	std::vector<AttachmentBuffer> m_attachments;
};
