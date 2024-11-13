#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <variant>
#include <vector>
#include <array>
#include <filesystem>
#include <map>
#include <type_traits>

#include "chill_renderer/assert.hpp"


namespace chill_renderer { 
namespace fs = std::filesystem;

inline constexpr int EMPTY_VBO = 0;
inline constexpr GLenum DEFAULT_TYPE = 0;
struct nulldata_t;

template<typename T>
constexpr decltype(auto) to_enum_elem_type(T enumerator) noexcept;
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
	MSAA_TEXTURE,
	MSAA_RENDER_BUFFER,
	NONE
};

enum class TextureType {
	GENERIC,
	DIFFUSE,
	SPECULAR,
	EMISSION,
	DEPTH,
	DEPTH_STENCIL,
	NONE,
};

enum class TextureWrap {
	REPEAT,
	CLAMP_EDGE,
	CLAMP_BORDER,

	MIRROR_REPEAT,

	NONE,
};
auto conv_wrap(TextureWrap a_wrap) noexcept -> GLenum;

enum class TextureFilter {
	LINEAR,
	NEAREST, 
	MIPMAP_LINEAR,
	MIPMAP_NEAREST,

	NONE,
};
auto conv_filter(TextureFilter a_filter, GLuint& tex_min, GLuint& tex_mag) noexcept -> void;

enum class TextureCmpFunc {
	LEQUAL,
	GEQUAL,
	LESS,
	GREATER,
	EQUAL,
	NOTEQUAL,
	ALWAYS,
	NEVER,

	NONE,
};
auto conv_cmp_func(TextureCmpFunc a_cmp) noexcept -> GLuint;

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
	Texture(const Texture& a_texture);
	Texture(Texture&& a_texture) noexcept;
	~Texture();

	auto operator=(const Texture& a_texture) -> Texture&;
	auto operator=(Texture&& a_texture) noexcept -> Texture&;

	virtual auto activate() const noexcept -> void = 0;
	virtual auto set_border_color(const glm::vec3& a_border_color) -> void = 0;
	virtual auto set_wrap(TextureWrap a_wrap) -> void = 0;
	virtual auto set_filter(TextureFilter a_filter) -> void = 0;
	virtual auto set_cmp_func(TextureCmpFunc a_cmp_func) -> void = 0;

	auto set_unit_id(int a_unit_id) noexcept -> void;
	auto set_type(TextureType a_type) noexcept -> void;

	auto get_id() const noexcept -> GLuint;
	auto get_type() const noexcept -> TextureType;
	auto get_wrap() const noexcept -> TextureWrap;
	auto get_cmp_func() const noexcept -> TextureCmpFunc;
	auto get_filter() const noexcept -> TextureFilter;
	auto get_unit_id() const noexcept -> int; 

protected:
	auto refcnt_dec() -> void;
	auto refcnt_inc() -> void;

	GLuint m_id = EMPTY_VBO;
	TextureWrap m_wrap = TextureWrap::NONE;
	TextureFilter m_filter = TextureFilter::NONE;
	TextureCmpFunc m_cmp = TextureCmpFunc::NONE;

private:
	TextureType m_type = TextureType::NONE;
	int m_unit_id = 0;
};

class Texture2D : public Texture {
public:
	Texture2D() = default;
	Texture2D(TextureType a_type, std::wstring a_path, bool a_flip_image, bool a_gamma_correction, GLenum a_data_type = GL_NONE);
	template<typename T = nulldata_t>
	Texture2D(TextureType a_type, int width, int height, GLenum a_data_type = DEFAULT_TYPE, T* a_data = nullptr);

	auto activate() const noexcept -> void;
	auto set_border_color(const glm::vec3& a_border_color) -> void;
	auto set_wrap(TextureWrap a_wrap) -> void;
	auto set_filter(TextureFilter a_filter) -> void;
	auto set_cmp_func(TextureCmpFunc a_cmp_func) -> void;

	auto get_filename() const noexcept -> std::wstring;
	auto get_path() const noexcept -> std::wstring;
	auto is_flipped() const noexcept -> bool;
	auto is_gamma_corr() const noexcept -> bool;

private:
	std::wstring m_path = L"";
	// TODO: int width{};
	// TODO: int height{};
	bool m_flipped = false;
	bool m_gamma_corr = true;
};

class Texture3D : public Texture {
public:
	Texture3D() = default;

	auto activate() const noexcept -> void;
	auto set_border_color(const glm::vec3& a_border_color) -> void;
	auto set_wrap(TextureWrap a_wrap) -> void;
	auto set_filter(TextureFilter a_filter) -> void;
	auto set_cmp_func(TextureCmpFunc a_cmp_func) -> void;

private:
};

// TODO: 6 Texture2D?
class TextureCubemap : public Texture {
public:
	TextureCubemap() = default;
	TextureCubemap(TextureType a_type, std::vector<std::wstring> a_paths, bool a_flip_images, bool a_gamma_correction, GLenum a_data_type = DEFAULT_TYPE);
	template<typename T = nulldata_t>
	TextureCubemap(TextureType a_type, int a_width, int a_height, GLenum a_data_type = DEFAULT_TYPE, const std::array<T*, 6>& a_data = std::array<nulldata_t*, 6>{});

	auto activate() const noexcept -> void;
	auto set_border_color(const glm::vec3& a_border_color) -> void;
	auto set_wrap(TextureWrap a_wrap) -> void;
	auto set_filter(TextureFilter a_filter) -> void;
	auto set_cmp_func(TextureCmpFunc a_cmp_func) -> void;

	auto get_paths() const noexcept -> std::vector<std::wstring>;
	auto get_filenames() const noexcept -> std::vector<std::wstring>;

private:
	std::vector<std::wstring> m_paths{};
	bool m_flipped = false;
	bool m_gamma_corr = true;
};

class TextureMSAA : public Texture {
public:
	TextureMSAA() = default;
	TextureMSAA(TextureType a_type, int a_width, int a_height, int a_samples);

	auto activate() const noexcept -> void;
	auto set_border_color(const glm::vec3& a_border_color) -> void;
	auto set_wrap(TextureWrap a_wrap) -> void;
	auto set_filter(TextureFilter a_filter) -> void;
	auto set_cmp_func(TextureCmpFunc a_cmp_func) -> void;

	auto get_samples() const noexcept -> int;

private:
	int m_samples = 0;
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

using uPtrTex = std::unique_ptr<Texture>;
using uPtrRen = std::unique_ptr<RenderBuffer>;

class AttachmentBuffer {
public:
	AttachmentBuffer() = default;
	AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples = 1);

	auto activate() const noexcept -> void;
	auto get_type() const noexcept -> AttachmentType;
	auto get_buf_type() const noexcept -> AttachmentBufferType;
	auto get_attachment() noexcept -> std::variant<uPtrTex, uPtrRen>&;

private:
	std::variant<uPtrTex, uPtrRen> m_attachment;
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

	auto bind() const noexcept -> void;
	auto unbind() const noexcept -> void;
	auto attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type, int a_samples = 1) -> void;
	auto attach_cubemap_face(GLenum a_cubemap_face) -> void;
	auto activate_color() const noexcept -> void;
	auto activate_depth() const noexcept -> void;
	auto check_status() const noexcept -> bool;

	auto set_samples(int a_samples) -> bool;
	auto set_resolution(int a_width, int a_height) -> void;

	auto get_id() const noexcept -> GLuint;
	auto get_width() const noexcept -> int;
	auto get_height() const noexcept -> int;
	auto get_samples() const noexcept -> int;
	auto get_color_attachment_buffer() noexcept -> AttachmentBuffer&;
	auto get_depth_attachment_buffer() noexcept -> AttachmentBuffer&;
	auto get_depth_stencil_attachment_buffer() noexcept -> AttachmentBuffer&;

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
	auto clear() -> void;

	auto set_binding_point(int a_binding_point) noexcept -> void;

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
