#pragma once

#include <variant>
#include <vector>
#include <memory>
#include <filesystem>

#include "chill_engine/assert.hpp"

constexpr int EMPTY_VBO = 0;
constexpr int ATTRIB_POS_LOCATION    = 0;
constexpr int ATTRIB_TEX_LOCATION    = 1;
constexpr int ATTRIB_NORMAL_LOCATION = 2;
constexpr int ATTRIB_COLOR_LOCATION  = 3; 

std::filesystem::path guess_path(std::wstring a_path);
 
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

struct BufferObjects {
	BufferObjects() = default;
	BufferObjects(const BufferObjects& a_obj);
	BufferObjects(BufferObjects&& a_obj);
	~BufferObjects();

	auto operator=(BufferObjects& a_obj) -> BufferObjects&;
	auto operator=(BufferObjects&& a_obj) -> BufferObjects&;

	unsigned int VAO = EMPTY_VBO;
	unsigned int VBO_UVs = EMPTY_VBO;
	unsigned int VBO_pos = EMPTY_VBO;
	unsigned int VBO_normals = EMPTY_VBO;
	unsigned int EBO = EMPTY_VBO;
};


class Texture {
public:
    Texture(std::wstring a_path, TextureType a_type, bool a_flip_image, int texture_unit);
    Texture(int a_width, int a_height, TextureType a_type);
	Texture(const Texture& a_texture);
	Texture(Texture&& a_texture); 
    ~Texture(); 

	auto operator=(const Texture& a_texture) -> Texture&;
	auto operator=(Texture&& a_texture) -> Texture&;

	auto set_unit_id(int a_unit_id) -> void;
	auto set_type(TextureType a_type) -> void;
    auto activate() const -> void; 

    auto get_id() const -> unsigned int;
    auto get_path() const -> std::wstring;
    auto get_filename() const -> std::wstring;
    auto get_type() const -> TextureType;
	auto get_unit_id() const -> int;

	auto is_flipped() const -> bool;

private:
    unsigned m_id = EMPTY_VBO;
    std::wstring m_path = L"";
	std::wstring m_filename = L"";
    TextureType m_type = TextureType::NONE;
    int m_unit_id = 0;
	bool m_flipped = false;
};

class RenderBuffer {
public:
	RenderBuffer() = default; 
	RenderBuffer(int a_width, int a_height, RenderBufferType a_type); 
	RenderBuffer(const RenderBuffer& a_ren_buf);
	RenderBuffer(RenderBuffer&& a_ren_buf);
	auto operator=(const RenderBuffer& a_ren_buf) -> RenderBuffer&;
	auto operator=(RenderBuffer&& a_ren_buf) -> RenderBuffer&;
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
	std::variant<std::shared_ptr<Texture>, std::shared_ptr<RenderBuffer>> m_attachment;
	AttachmentType m_type = AttachmentType::NONE;
};

class Framebuffer { 
public:
	Framebuffer(int a_width, int a_height); 
	Framebuffer(const Framebuffer& a_frame_buf);
	Framebuffer(Framebuffer&& a_frame_buf);
	~Framebuffer(); 

	auto operator=(const Framebuffer& a_frame_buf) -> Framebuffer&;
	auto operator=(Framebuffer&& a_frame_buf) -> Framebuffer&;

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
