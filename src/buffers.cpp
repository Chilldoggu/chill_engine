#include <glad/glad.h>

#include <memory>
#include <vector>
#include <variant>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <filesystem>

#include "buffers.hpp"
#include "assert.hpp"
#include "file_manager.hpp"


namespace fs = std::filesystem;

fs::path get_proj_path() {
	fs::path p = fs::current_path();
	if (p.filename() == "build" || p.filename() == "install") {
		p = p.parent_path();
	}

	return p;
}

 
std::ostream& operator<<(std::ostream& os, const TextureType& a_type) {
	std::string str_type;
	switch (a_type) {
	case TextureType::COLOR         : str_type = "COLOR";         break;
	case TextureType::DEPTH         : str_type = "DEPTH";         break;
	case TextureType::DIFFUSE       : str_type = "DIFFUSE";       break;
	case TextureType::SPECULAR      : str_type = "SPECULAR";      break;
	case TextureType::EMISSION      : str_type = "EMISSION";      break;
	case TextureType::DEPTH_STENCIL : str_type = "DEPTH_STENCIL"; break;
	case TextureType::NONE          : str_type = "NONE";          break;
	default: str_type = "UNKNOWN"; break;
	}
	os << str_type; 
	return os; 
}

Texture::Texture(std::wstring a_dir, TextureType a_type, int a_texture_unit) {
	load_texture(a_dir, a_type, a_texture_unit);
}

Texture::Texture(int a_width, int a_height, TextureType a_type) {
	create_texture(a_width, a_height, a_type);
}

Texture::~Texture() {
	std::cout << "DELETING: " << wstos(m_dir) << std::endl;
	if (m_type != TextureType::NONE) {
		glDeleteTextures(1, &m_texture_id); 
	}
}

void Texture::clear() {
	glDeleteTextures(1, &m_texture_id);
    m_type = TextureType::NONE;
    m_texture_unit = 0;
    m_dir = L"";
}

void Texture::set_texture_unit(int a_unit_id) {
	m_texture_unit = a_unit_id;
}

void Texture::set_texture_type(TextureType a_type) {
	m_type = a_type;
}

void Texture::create_texture(int a_width, int a_height, TextureType a_type) {
	if (a_type != TextureType::COLOR && a_type != TextureType::DEPTH && a_type != TextureType::DEPTH_STENCIL)
		ERROR("[TEXTURE::GEN_FBO_TEXTURE] Bad texture type", Error_action::throwing); 

	clear(); 
	m_type = a_type;

	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available 

	switch (a_type) {
	case TextureType::COLOR:         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,              a_width, a_height, 0, GL_RGB,              GL_UNSIGNED_BYTE, NULL);     break;
	case TextureType::DEPTH:         glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  a_width, a_height, 0, GL_DEPTH_COMPONENT,  GL_UNSIGNED_BYTE, NULL);     break;
	case TextureType::DEPTH_STENCIL: glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, a_width, a_height, 0, GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8, NULL); break;
	}
}

void Texture::load_texture(std::wstring a_dir, TextureType a_type, int a_texture_unit, bool a_flip_UVs) {
	if (a_type != TextureType::DIFFUSE && a_type != TextureType::SPECULAR && a_type != TextureType::EMISSION)
		ERROR("[TEXTURE::LOAD_TEXTURE] Bad texture type", Error_action::throwing); 

	clear();
	m_dir = a_dir;
	m_type = a_type;
	m_texture_unit = a_texture_unit;

	fs::path p = get_proj_path();
	p /= a_dir;

	glGenTextures(1, &m_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available

	int nrChannels{ };
	stbi_set_flip_vertically_on_load(a_flip_UVs);
	int width{ };
	int height{ };
	unsigned char* data = stbi_load(p.string().c_str(), &width, &height, &nrChannels, 0);

	unsigned format;
	if (nrChannels == 3) {
		format = GL_RGB;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else if (nrChannels == 4) { 
		format = GL_RGBA;

		// Interpolation of transparent borders with GL_REPEAT colors them
		// with opposite border color. With GL_CLAMP_TO_EDGE we make sure to 
		// spread transparency across the edges.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		ERROR(std::format("[TEXTURE::LOAD_TEXTURE] Unsupported texture format for {} number of channels.", nrChannels), Error_action::throwing);
	}

	// INFO: Testing
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		ERROR(std::format("[TEXTURE::LOAD_TEXTURE] Couldn't load texture data {}", wstos(m_dir)), Error_action::throwing);
	}

	stbi_image_free(data);
}

void Texture::activate() const {
	if (m_type != TextureType::NONE) {
		glActiveTexture(GL_TEXTURE0 + m_texture_unit);
		glBindTexture(GL_TEXTURE_2D, m_texture_id);
	}
}

TextureType Texture::get_type() const {
	return m_type;
}

std::wstring Texture::get_dir() const {
	return m_dir;
}

unsigned int Texture::get_id() const {
	return m_texture_id;
}

int Texture::get_unit_id() const {
	return m_texture_unit;
}

RenderBuffer::RenderBuffer(int a_width, int a_height, RenderBufferType a_type) :m_type{ a_type } {
	glGenRenderbuffers(1, &m_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rbo); 

	switch (a_type) { 
	case RenderBufferType::COLOR:         glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB,  a_width, a_height); break;
	case RenderBufferType::DEPTH:         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, a_width, a_height); break;
	case RenderBufferType::DEPTH_STENCIL: glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, a_width, a_height); break;
	default:
		ERROR("[RENDERBUFFER::RENDERBUFFER] Bad renderbuffer type.", Error_action::throwing);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer() {
	if (m_type != RenderBufferType::NONE) {
		glDeleteRenderbuffers(1, &m_rbo); 
	}
}

unsigned int RenderBuffer::get_id() const {
	return m_rbo;
}

RenderBufferType RenderBuffer::get_type() const {
	return m_type;
} 
AttachmentBuffer::AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type) :m_type{ a_attach_type } {
	if (a_buf_type == AttachmentBufferType::TEXTURE) {
		switch (a_attach_type) {
		case AttachmentType::COLOR:         m_attachment = std::make_shared<Texture>(a_width, a_height, TextureType::COLOR); break;
		case AttachmentType::DEPTH:         m_attachment = std::make_shared<Texture>(a_width, a_height, TextureType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = std::make_shared<Texture>(a_width, a_height, TextureType::DEPTH_STENCIL); break; 
		}
	} else if (a_buf_type == AttachmentBufferType::RENDER_BUFFER) {
		switch (a_attach_type) {
		case AttachmentType::COLOR:         m_attachment = std::make_shared<RenderBuffer>(a_width, a_height, RenderBufferType::COLOR); break;
		case AttachmentType::DEPTH:         m_attachment = std::make_shared<RenderBuffer>(a_width, a_height, RenderBufferType::DEPTH); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = std::make_shared<RenderBuffer>(a_width, a_height, RenderBufferType::DEPTH_STENCIL); break; 
		} 
	} else { 
		ERROR("[ATTACHMENTBUFFER::ATTACHMENTBUFFER] Bad attachment buffer type.", Error_action::throwing);
	}
} 

void AttachmentBuffer::activate() const {
	if (std::holds_alternative<std::shared_ptr<Texture>>(m_attachment)) {
		std::get<std::shared_ptr<Texture>>(m_attachment)->activate();
	} else if (std::holds_alternative<std::shared_ptr<RenderBuffer>>(m_attachment)) {
		// WARNING: I dunno, do it later
	}
}

AttachmentType AttachmentBuffer::get_type() const {
	return m_type;
}

std::variant<std::shared_ptr<Texture>, std::shared_ptr<RenderBuffer>> AttachmentBuffer::get_attachment() const {
	return m_attachment;
}

Framebuffer::Framebuffer(int a_width, int a_height) :m_width{ a_width }, m_height{ a_height } {
	glGenFramebuffers(1, &m_fbo);
}

Framebuffer::~Framebuffer() {
	glDeleteFramebuffers(1, &m_fbo);
}

void Framebuffer::attach(AttachmentType a_attach_type, AttachmentBufferType a_buf_type) { 
	AttachmentBuffer new_attachment(m_width, m_height, a_attach_type, a_buf_type);
	m_attachments.push_back(new_attachment); 
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	if (a_buf_type == AttachmentBufferType::TEXTURE) {
		std::shared_ptr<Texture> tex = std::get<std::shared_ptr<Texture>>(new_attachment.get_attachment());
		switch (a_attach_type) {
		case AttachmentType::COLOR:         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,        GL_TEXTURE_2D, tex->get_id(), 0); break;
		case AttachmentType::DEPTH:         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,         GL_TEXTURE_2D, tex->get_id(), 0); break;
		case AttachmentType::DEPTH_STENCIL: glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tex->get_id(), 0); break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] Wrong attachment type.", Error_action::throwing);
		} 
	} else if (a_buf_type == AttachmentBufferType::RENDER_BUFFER) {
		std::shared_ptr<RenderBuffer> ren_buf = std::get<std::shared_ptr<RenderBuffer>>(new_attachment.get_attachment());
		switch (a_attach_type) {
		case AttachmentType::COLOR:         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,        GL_RENDERBUFFER, ren_buf->get_id()); break;
		case AttachmentType::DEPTH:         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,         GL_RENDERBUFFER, ren_buf->get_id()); break;
		case AttachmentType::DEPTH_STENCIL: glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ren_buf->get_id()); break;
		default:
			ERROR("[FRAMEBUFFER::ATTACH] Wrong attachment type.", Error_action::throwing);
		} 
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

AttachmentBuffer Framebuffer::get_attachment_buffer(AttachmentType a_type) const { 
	AttachmentBuffer ret_attachment_buffer;
	for (auto& attachment_buffer : m_attachments) {
		auto attachment = attachment_buffer.get_attachment(); 

		if (attachment_buffer.get_type() == a_type) { 
			ret_attachment_buffer = attachment_buffer;
			break;
		}
	} 

	return ret_attachment_buffer; 
}

void Framebuffer::activate_color() const {
	get_attachment_buffer(AttachmentType::COLOR).activate();
}

unsigned int Framebuffer::get_id() const {
	return m_fbo;
} 

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::check_status() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false; 
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);  	
	return true;
}


void Framebuffer::set_width(int a_width) {
	m_width = a_width;
}

void Framebuffer::set_height(int a_height) {
	m_height = a_height;
}

int Framebuffer::get_width() const {
	return m_width;
}

int Framebuffer::get_height() const {
	return m_height;
}
