#pragma once


#include "glad/glad.h"
#include "glm/glm.hpp" // template definitions
#include "glm/gtc/type_ptr.hpp"  // template definitions

#include <variant>
#include <vector>
#include <filesystem>
#include <map>
#include <type_traits>

#include "chill_renderer/assert.hpp" // template definitions


namespace chill_renderer { 
namespace fs = std::filesystem;

inline constexpr int EMPTY_VBO = 0;

fs::path guess_path(const std::wstring& a_path);

template<typename T>
constexpr decltype(auto) to_enum_elem_type(T enumerator) noexcept;

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
	MSAA_TEXTURE,
	MSAA_RENDER_BUFFER,
	NONE
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

	auto operator=(const BufferObjects& a_obj) -> BufferObjects&;
	auto operator=(BufferObjects&& a_obj) noexcept -> BufferObjects&;

	GLuint VAO = EMPTY_VBO;
	GLuint VBO_UVs = EMPTY_VBO;
	GLuint VBO_pos = EMPTY_VBO;
	GLuint VBO_normals = EMPTY_VBO;
	GLuint EBO = EMPTY_VBO;
private:
	auto refcnt_dec() -> void;
};

class Texture {
public:
	Texture() = default;
	Texture(std::wstring a_path, TextureType a_type, int texture_unit, bool a_flip_image, bool a_gamma_correction);
	Texture(std::vector<std::wstring> a_paths, int texture_unit, bool a_flip_images, bool a_gamma_correction);
	Texture(int a_width, int a_height, TextureType a_type);
	Texture(int a_width, int a_height, int a_samples, TextureType a_type);
	Texture(const Texture& a_texture);
	Texture(Texture&& a_texture) noexcept;
	~Texture();

	auto operator=(const Texture& a_texture) -> Texture&;
	auto operator=(Texture&& a_texture) noexcept -> Texture&;

	auto set_unit_id(int a_unit_id) noexcept -> void;
	auto set_type(TextureType a_type) noexcept -> void;
	auto activate() const noexcept -> void;

	auto get_id() const noexcept -> GLuint;
	auto get_path() const noexcept -> std::wstring;
	auto get_filename() const noexcept -> std::wstring;
	auto get_filenames() const noexcept -> std::vector<std::wstring>;
	auto get_type() const noexcept -> TextureType;
	auto get_unit_id() const noexcept -> int; 
	auto is_flipped() const noexcept -> bool;
	auto is_gamma_corr() const noexcept -> bool;

private:
	auto refcnt_dec() -> void;

	GLuint m_id = EMPTY_VBO;
	std::wstring m_path = L"";
	std::wstring m_filename = L"";
	std::vector<std::wstring> m_filenames{};
	TextureType m_type = TextureType::NONE;
	int m_unit_id = 0;
	int m_samples = 1;
	bool m_flipped = false;
	bool m_gamma_corr = true;
};

class RenderBuffer {
public:
	RenderBuffer() = default;
	RenderBuffer(int a_width, int a_height, RenderBufferType a_type);
	RenderBuffer(int a_width, int a_height, int a_samples, RenderBufferType a_type);
	RenderBuffer(const RenderBuffer& a_ren_buf);
	RenderBuffer(RenderBuffer&& a_ren_buf) noexcept;
	~RenderBuffer();

	auto operator=(const RenderBuffer& a_ren_buf) -> RenderBuffer&;
	auto operator=(RenderBuffer&& a_ren_buf) noexcept -> RenderBuffer&;

	auto get_id() const noexcept -> GLuint;
	auto get_type() const noexcept -> RenderBufferType;

private:
	auto refcnt_dec() -> void;

	GLuint m_rbo = EMPTY_VBO;
	RenderBufferType m_type = RenderBufferType::NONE;
	int m_samples = 1;
};

class AttachmentBuffer {
public:
	AttachmentBuffer() = default;
	AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples = 1);

	auto activate() const noexcept -> void;
	auto get_type() const noexcept -> AttachmentType;
	auto get_buf_type() const noexcept -> AttachmentBufferType;
	auto get_attachment() const noexcept -> std::variant<Texture, RenderBuffer>;

private:
	std::variant<Texture, RenderBuffer> m_attachment;
	AttachmentType m_type = AttachmentType::NONE;
	AttachmentBufferType m_buf_type = AttachmentBufferType::NONE;
};

// Move-only because set_id()/attach() alters attachments which would not be
// reflected on copies.
class FrameBuffer {
public:
	FrameBuffer() = default;
	FrameBuffer(int a_width, int a_height);
	FrameBuffer(FrameBuffer&& a_frame_buf) noexcept;
	FrameBuffer(const FrameBuffer& a_frame_buf) = delete;
	~FrameBuffer();

	auto operator=(FrameBuffer&& a_frame_buf) noexcept -> FrameBuffer&; 
	auto operator=(const FrameBuffer& a_frame_buf) -> FrameBuffer& = delete;

	auto attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples = 1) -> void;
	auto attach_cubemap_face(GLenum a_cubemap_face) -> void;
	auto get_attachment_buffer(AttachmentType a_type) const noexcept -> AttachmentBuffer;
	auto activate_color() const noexcept -> void;
	auto get_id() const noexcept -> GLuint;
	auto bind() const noexcept -> void;
	auto unbind() const noexcept -> void;
	auto check_status() const noexcept -> bool;
	auto set_width(int a_width) noexcept -> void;
	auto set_height(int a_height) noexcept -> void;
	auto set_samples(int a_samples) -> bool;
	auto get_width() const noexcept -> int;
	auto get_height() const noexcept -> int;
	auto get_samples() const noexcept -> int;

private:
	auto refcnt_dec() -> void;

	int m_width = 0;
	int m_height = 0;
	int m_samples = 1;
	GLuint m_fbo = EMPTY_VBO;
	AttachmentBuffer m_color_attachment{};
	AttachmentBuffer m_depth_attachment{};
	AttachmentBuffer m_depth_stencil_attachment{};
};

struct UniformBufferElement {
	int m_size{};
	int m_base_alignment{};
	int m_offset_alignment{};
	GLuint m_UBO_id{ EMPTY_VBO };

	template<typename T>
	auto operator=(const T& a_value) -> UniformBufferElement&;
};

// Implementation supports std140 memory layout. All uniform types from
// uniform block should be set in order from top to bottom (<float, glm::vec3, int[10]> and so on).
class UniformBuffer {
private:
	struct EmptyType {};

public: 
	using NameUniMap =
		std::map<std::string, UniformBufferElement>;

	UniformBuffer() = default;
	UniformBuffer(const UniformBuffer& a_uni_buf); 
	UniformBuffer(UniformBuffer&& a_uni_buf) noexcept; 
	~UniformBuffer();

	UniformBuffer& operator=(const UniformBuffer& a_uni_buf);
	UniformBuffer& operator=(UniformBuffer&& a_uni_buf) noexcept; 
	UniformBufferElement& operator[](const std::string& a_uniform_name);

	template<typename T>
	auto push_element(const std::string& a_uniform_name) -> void;
	template<typename U = EmptyType, typename... T>
	void push_elements(const std::vector<std::string>& a_uniform_names);
	auto check_status() const noexcept -> bool; 
	auto create_buffer() noexcept -> void; 
	auto set_binding_point(int a_binding_point) noexcept -> void;
	auto clear() -> void;

	NameUniMap get_elements() const noexcept; 

private: 
	auto refcnt_dec() -> void;
	auto refcnt_inc() -> void;

	template<typename T>
	auto get_size_and_base_alignment();

	int m_size{}; // bytes
	int m_binding_point{-1};
	GLuint m_id{ EMPTY_VBO };
	NameUniMap m_elements{};
}; 

// Template definitions
#include "buffers_templates.cpp"
}
