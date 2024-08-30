#define STB_IMAGE_IMPLEMENTATION

#include <stb_image/stb_image.h>

#include <memory>
#include <vector>
#include <variant>
#include <filesystem>

#include "chill_engine/buffers.hpp"
#include "chill_engine/assert.hpp"
#include "chill_engine/file_manager.hpp" 
#include "chill_engine/application.hpp"

namespace fs = std::filesystem;

fs::path guess_path(std::wstring a_path) {
	// Guess project directory
	fs::path guessed_proj_dir = fs::current_path();
	if (guessed_proj_dir.filename() == "build" || guessed_proj_dir.filename() == "install") {
		guessed_proj_dir = guessed_proj_dir.parent_path();
	}

	// Check if path is good
	fs::path ret_path(a_path);
	if (!(fs::exists(ret_path) && fs::is_regular_file(ret_path) && !fs::is_empty(ret_path))) {
		ret_path = guessed_proj_dir / ret_path;
		if (!(fs::exists(ret_path) && fs::is_regular_file(ret_path) && !fs::is_empty(ret_path)))
			return fs::path("");
	}

	return ret_path;
}

BufferObjects::BufferObjects(const BufferObjects& a_obj) { 
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::MESHES, a_obj.VAO); 

	VAO = a_obj.VAO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
	EBO = a_obj.EBO; 
}

BufferObjects::BufferObjects(BufferObjects&& a_obj) {
	VAO = a_obj.VAO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
	EBO = a_obj.EBO; 

	a_obj.VAO = EMPTY_VBO;
	a_obj.VBO_UVs = EMPTY_VBO;
	a_obj.VBO_pos = EMPTY_VBO;
	a_obj.VBO_normals = EMPTY_VBO;
	a_obj.EBO = EMPTY_VBO;
}

BufferObjects& BufferObjects::operator=(BufferObjects& a_obj) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::MESHES, a_obj.VAO); 

	VAO = a_obj.VAO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
	EBO = a_obj.EBO; 
	
	return *this;
}

BufferObjects& BufferObjects::operator=(BufferObjects&& a_obj) {
	VAO = a_obj.VAO;
	VBO_UVs = a_obj.VBO_UVs;
	VBO_pos = a_obj.VBO_pos;
	VBO_normals = a_obj.VBO_normals;
	EBO = a_obj.EBO; 

	a_obj.VAO = EMPTY_VBO;
	a_obj.VBO_UVs = EMPTY_VBO;
	a_obj.VBO_pos = EMPTY_VBO;
	a_obj.VBO_normals = EMPTY_VBO;
	a_obj.EBO = EMPTY_VBO; 

	return *this;
}

BufferObjects::~BufferObjects() {
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

Texture::Texture(std::wstring a_path, TextureType a_type, bool a_flip_image, int a_unit_id)
	:m_type{ a_type }, m_unit_id{ a_unit_id }, m_flipped{ a_flip_image }
{
	if (a_type != TextureType::DIFFUSE && a_type != TextureType::SPECULAR && a_type != TextureType::EMISSION)
		ERROR("[TEXTURE::LOAD_TEXTURE] Bad texture type", Error_action::throwing); 
 
	fs::path p = guess_path(a_path);
	if (p == fs::path())
		ERROR(std::format("[TEXTURE::TEXTURE] Bad texture path: {}", wstos(a_path)), Error_action::throwing);

	m_path = fs::canonical(p).wstring();
	m_filename = p.filename().wstring();

	glGenTextures(1, &m_id); 
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available

	int nrChannels{ };
	int width{ };
	int height{ };
	stbi_set_flip_vertically_on_load(a_flip_image);
	unsigned char* data = stbi_load(wstos(m_path).c_str(), &width, &height, &nrChannels, 0);

	unsigned format;
	if (nrChannels == 3) {
		format = GL_RGB;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	} else if (nrChannels == 4) { 
		format = GL_RGBA;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		ERROR(std::format("[TEXTURE::LOAD_TEXTURE] Unsupported texture format {} with {} number of channels.", wstos(m_path), nrChannels), Error_action::throwing);
	}

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		ERROR(std::format("[TEXTURE::LOAD_TEXTURE] Couldn't load texture data {}", wstos(m_path)), Error_action::throwing);
	}

	stbi_image_free(data);
}

Texture::Texture(int a_width, int a_height, TextureType a_type) :m_type{ a_type } { 
	if (a_type != TextureType::COLOR && a_type != TextureType::DEPTH && a_type != TextureType::DEPTH_STENCIL)
		ERROR("[TEXTURE::GEN_FBO_TEXTURE] Bad texture type", Error_action::throwing); 

	glGenTextures(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, m_id); 
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_NEAREST variations available
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // GL_NEAREST available 

	switch (a_type) {
	case TextureType::COLOR:         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,              a_width, a_height, 0, GL_RGB,              GL_UNSIGNED_BYTE, NULL);     break;
	case TextureType::DEPTH:         glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  a_width, a_height, 0, GL_DEPTH_COMPONENT,  GL_UNSIGNED_BYTE, NULL);     break;
	case TextureType::DEPTH_STENCIL: glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, a_width, a_height, 0, GL_DEPTH_STENCIL,    GL_UNSIGNED_INT_24_8, NULL); break;
	}
}

Texture::Texture(const Texture& a_texture) { 
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, a_texture.m_id); 

	m_path = a_texture.m_path;
	m_filename = a_texture.m_filename;
	m_type = a_texture.m_type;
	m_id = a_texture.m_id;
	m_unit_id = a_texture.m_unit_id;
}

// When moving an object, reference count shouldn't increment.
Texture::Texture(Texture&& a_texture) {
	m_path = a_texture.m_path;
	m_filename = a_texture.m_filename;
	m_type = a_texture.m_type;
	m_id = a_texture.m_id;
	m_unit_id = a_texture.m_unit_id; 

    a_texture.m_path = L"";
	a_texture.m_filename = L"";
    a_texture.m_unit_id = 0;
	a_texture.m_id = EMPTY_VBO;
    a_texture.m_type = TextureType::NONE;
}

Texture& Texture::operator=(const Texture& a_texture) { 
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::TEXTURES, a_texture.m_id); 

	m_path = a_texture.m_path;
	m_filename = a_texture.m_filename;
	m_type = a_texture.m_type;
	m_id = a_texture.m_id;
	m_unit_id = a_texture.m_unit_id;

	return *this;
}

// When moving an object, reference count shouldn't increment.
Texture& Texture::operator=(Texture&& a_texture) {
	m_path = a_texture.m_path;
	m_filename = a_texture.m_filename;
	m_type = a_texture.m_type;
	m_id = a_texture.m_id;
	m_unit_id = a_texture.m_unit_id; 

    a_texture.m_path = L"";
	a_texture.m_filename = L"";
    a_texture.m_unit_id = 0;
	a_texture.m_id = EMPTY_VBO;
    a_texture.m_type = TextureType::NONE;

	return *this;
}

Texture::~Texture() {
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::TEXTURES, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::TEXTURES, m_id)) {
			std::cout << "DELETING: " << wstos(m_path) << std::endl;
			glDeleteTextures(1, &m_id); 
		} 
	}
}

void Texture::set_unit_id(int a_unit_id) {
	m_unit_id = a_unit_id;
}

void Texture::set_type(TextureType a_type) {
	m_type = a_type;
} 

void Texture::activate() const {
	if (m_type != TextureType::NONE) {
		glActiveTexture(GL_TEXTURE0 + m_unit_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
	}
}

TextureType Texture::get_type() const {
	return m_type;
}

std::wstring Texture::get_path() const {
	return m_path;
}

std::wstring Texture::get_filename() const {
	return m_filename;
}

unsigned Texture::get_id() const {
	return m_id;
}

int Texture::get_unit_id() const {
	return m_unit_id;
}

bool Texture::is_flipped() const {
	return m_flipped;
} 

RenderBuffer::RenderBuffer(int a_width, int a_height, RenderBufferType a_type) :m_type{ a_type } {
	glGenRenderbuffers(1, &m_rbo);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, m_rbo);
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

RenderBuffer::RenderBuffer(const RenderBuffer& a_ren_buf) {
	Application::get_instance().get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, a_ren_buf.get_id());

	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type; 
}

RenderBuffer::RenderBuffer(RenderBuffer&& a_ren_buf) {
	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;

	a_ren_buf.m_rbo = EMPTY_VBO;
	a_ren_buf.m_type = RenderBufferType::NONE; 
}

RenderBuffer& RenderBuffer::operator=(const RenderBuffer& a_ren_buf) {
	Application::get_instance().get_instance().get_rmanager().inc_ref_count(ResourceType::RENDER_BUFFERS, a_ren_buf.get_id());

	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;
	
	return *this;
}

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& a_ren_buf) {
	m_rbo = a_ren_buf.m_rbo;
	m_type = a_ren_buf.m_type;

	a_ren_buf.m_rbo = EMPTY_VBO;
	a_ren_buf.m_type = RenderBufferType::NONE; 

	return *this;
}

RenderBuffer::~RenderBuffer() {
	if (m_type != RenderBufferType::NONE) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::RENDER_BUFFERS, m_rbo);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::RENDER_BUFFERS, m_rbo)) {
			glDeleteRenderbuffers(1, &m_rbo); 
		}
	}
}

unsigned RenderBuffer::get_id() const {
	return m_rbo;
}

RenderBufferType RenderBuffer::get_type() const {
	return m_type;
} 

AttachmentBuffer::AttachmentBuffer(int a_width, int a_height, AttachmentType a_attach_type, AttachmentBufferType a_buf_type) :m_type{ a_attach_type } {
	ResourceManager& rman = Application::get_instance().get_rmanager();
	if (a_buf_type == AttachmentBufferType::TEXTURE) {
		switch (a_attach_type) {
		case AttachmentType::COLOR:         m_attachment = std::make_shared<Texture>(rman.create_texture(a_width, a_height, TextureType::COLOR)); break;
		case AttachmentType::DEPTH:         m_attachment = std::make_shared<Texture>(rman.create_texture(a_width, a_height, TextureType::DEPTH)); break;
		case AttachmentType::DEPTH_STENCIL: m_attachment = std::make_shared<Texture>(rman.create_texture(a_width, a_height, TextureType::DEPTH_STENCIL)); break; 
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
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
}

Framebuffer::Framebuffer(const Framebuffer& a_frame_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, a_frame_buf.m_fbo);

	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_attachments = a_frame_buf.m_attachments;
}

Framebuffer::Framebuffer(Framebuffer&& a_frame_buf) {
	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_attachments = a_frame_buf.m_attachments;

	a_frame_buf.m_fbo = EMPTY_VBO;
	a_frame_buf.m_width = 0;
	a_frame_buf.m_height = 0;
	a_frame_buf.m_attachments.clear(); 
}

Framebuffer& Framebuffer::operator=(const Framebuffer& a_frame_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::FRAME_BUFFERS, a_frame_buf.m_fbo);

	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_attachments = a_frame_buf.m_attachments;

	return *this;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& a_frame_buf) {
	m_fbo = a_frame_buf.m_fbo;
	m_width = a_frame_buf.m_width;
	m_height = a_frame_buf.m_height;
	m_attachments = a_frame_buf.m_attachments;

	a_frame_buf.m_fbo = EMPTY_VBO;
	a_frame_buf.m_width = 0;
	a_frame_buf.m_height = 0;
	a_frame_buf.m_attachments.clear(); 

	return *this;
}

Framebuffer::~Framebuffer() {
	if (m_fbo != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::FRAME_BUFFERS, m_fbo);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::FRAME_BUFFERS, m_fbo)) {
			glDeleteFramebuffers(1, &m_fbo);
		}
	}
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
		if (attachment_buffer.get_type() == a_type) { 
			return attachment_buffer;
		}
	} 

	return ret_attachment_buffer; 
}

void Framebuffer::activate_color() const {
	for (const AttachmentBuffer& attachment_buffer : m_attachments) {
		if (attachment_buffer.get_type() == AttachmentType::COLOR) { 
			attachment_buffer.activate();
			return;
		}
	} 
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
