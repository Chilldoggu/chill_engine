using namespace chill_renderer;

template<typename T>
constexpr decltype(auto) to_enum_elem_type(T enumerator) noexcept {
	return static_cast<std::underlying_type_t<T>>(enumerator);
}

template<typename T>
bool cmp_types(GLenum a_data_type) noexcept {
	if (
		(a_data_type == GL_INT            && !std::is_same_v<T, int>)           ||
		(a_data_type == GL_BYTE           && !std::is_same_v<T, char>)          ||
		(a_data_type == GL_SHORT          && !std::is_same_v<T, short>)         ||
		(a_data_type == GL_FLOAT          && !std::is_same_v<T, float>)         ||
		(a_data_type == GL_UNSIGNED_INT   && !std::is_same_v<T, unsigned int>)  ||
		(a_data_type == GL_UNSIGNED_BYTE  && !std::is_same_v<T, unsigned char>) ||
		(a_data_type == GL_UNSIGNED_SHORT && !std::is_same_v<T, unsigned short>)
		) {
		return false;
	}
	return true;
}

template<typename T>
Texture2D::Texture2D(TextureType a_type, int a_width, int a_height, GLenum a_data_type, T* a_data) 
{
	set_type(a_type);

	glGenTextures(1, &m_id);
	refcnt_inc();
	glBindTexture(GL_TEXTURE_2D, m_id); 

	GLint in_format{};
	GLenum format{}; 
	if (a_type == TextureType::GENERIC || a_type == TextureType::DIFFUSE || a_type == TextureType::SPECULAR || a_type == TextureType::EMISSION) {
		in_format = format = GL_RGB;
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_BYTE : a_data_type;
	}
	else if (a_type == TextureType::DEPTH) {
		in_format = format = GL_DEPTH_COMPONENT;
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_BYTE : a_data_type;
	}
	else if (a_type == TextureType::DEPTH_STENCIL) {
		in_format = GL_DEPTH24_STENCIL8;
		format = GL_DEPTH_STENCIL;
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_INT_24_8 : a_data_type;
	}
	else {
		ERROR("[TEXTURE2D::TEXTURE2D] Wrong TextureType.", Error_action::throwing);
	}

	if (std::is_same_v<T, nulldata_t>) {
		glTexImage2D(GL_TEXTURE_2D, 0, in_format, a_width, a_height, 0, format, a_data_type, NULL);
	}
	else if (cmp_types<T>(a_data_type)){
		glTexImage2D(GL_TEXTURE_2D, 0, in_format, a_width, a_height, 0, format, a_data_type, a_data);
	}
	else {
		ERROR("[TEXTURE2D::TEXTURE2D] Data pointer doesn't match selected data_type.", Error_action::throwing);
	}

	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::LINEAR);
}

template<typename T>
TextureCubemap::TextureCubemap(TextureType a_type, int a_width, int a_height, GLenum a_data_type, const std::array<T*, 6>& a_data) {
	set_type(a_type);

	glGenTextures(1, &m_id);
	refcnt_inc();
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id); 

	GLint in_format{};
	GLenum format{};
	if (a_type == TextureType::GENERIC || a_type == TextureType::DIFFUSE || a_type == TextureType::SPECULAR || a_type == TextureType::EMISSION) {
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_BYTE : a_data_type;
		in_format = GL_RGB;
		format = GL_RGB;
	}
	else if (a_type == TextureType::DEPTH) {
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_BYTE : a_data_type;
		in_format = GL_DEPTH_COMPONENT;
		format = GL_DEPTH_COMPONENT;
	}
	else if (a_type == TextureType::DEPTH_STENCIL) {
		a_data_type = (a_data_type == DEFAULT_TYPE) ? GL_UNSIGNED_INT_24_8 : a_data_type;
		in_format = GL_DEPTH24_STENCIL8;
		format = GL_DEPTH_STENCIL;
	} 
	else {
		ERROR("[TEXTURECUBEMAP::TEXTURECUBEMAP] Wrong TextureType.", Error_action::throwing);
	}

	if (std::is_same_v<T, nulldata_t>) {
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, in_format, a_width, a_height, 0, format, a_data_type, a_data[i]);
		}
	}
	else if(cmp_types<T>(a_data_type)) {
		for (int i = 0; i < 6; ++i) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, in_format, a_width, a_height, 0, format, a_data_type, a_data[i]);
		}
	}
	else {
		ERROR("[TEXTURECUBEMAP::TEXTURECUBEMAP] Data pointer doesn't match selected data_type.", Error_action::throwing);
	}


	set_wrap(TextureWrap::CLAMP_EDGE);
	set_filter(TextureFilter::LINEAR);
}

template<typename T>
UniformBufferElement& UniformBufferElement::operator=(const T& a_value) {
	if (m_UBO_id == EMPTY_VBO)
		return *this;

	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO_id);

	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, bool>) {
		glBufferSubData(GL_UNIFORM_BUFFER, m_offset_alignment, m_size, &a_value); 
	}
	else if constexpr (std::is_same_v<T, glm::vec1> || std::is_same_v<T, glm::vec2> || 
					   std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4> || 
					   std::is_same_v<T, glm::mat2> || std::is_same_v<T, glm::mat3> || 
					   std::is_same_v<T, glm::mat4>) {
		glBufferSubData(GL_UNIFORM_BUFFER, m_offset_alignment, m_size, glm::value_ptr(a_value)); 
	}
	else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<bool>>) {
		glBufferSubData(GL_UNIFORM_BUFFER, m_offset_alignment, m_size, a_value.data()); 
	}
	else if constexpr (std::is_bounded_array_v<T>) { 
		glBufferSubData(GL_UNIFORM_BUFFER, m_offset_alignment, m_size, a_value); 
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);  

	return *this;
}

template<typename T>
void UniformBuffer::push_element(const std::string& a_uniform_name) {
	if (m_id == EMPTY_VBO) {
		glGenBuffers(1, &m_id);
		refcnt_inc();
	}

	auto [size, base_alignment] = get_size_and_base_alignment<T>();

	if (int off = m_size % base_alignment; off != 0)
		m_size += base_alignment - off;

	UniformBufferElement new_element{
		.m_size = size,
		.m_base_alignment = base_alignment,
		.m_offset_alignment = m_size,
		.m_UBO_id = m_id,
	};
	m_size += size;

	m_elements[a_uniform_name] = new_element; 
}

template<typename U, typename... T>
void UniformBuffer::push_elements(const std::vector<std::string>& a_uniform_names) {
	if constexpr (std::is_same_v<U, EmptyType>) {
		create_buffer();
		return; 
	}

	push_element<U>(a_uniform_names.at(0));
	push_elements<T...>(std::vector(a_uniform_names.begin() + 1, a_uniform_names.end())); 
}

template<typename T>
auto UniformBuffer::get_size_and_base_alignment() {
	int base_alignment{};
	int size{}; 

	if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, bool>) {
		base_alignment = 4;
		size = 4;
	}
	else if constexpr (std::is_same_v<T, glm::vec1> || std::is_same_v<T, glm::vec2>) {
		base_alignment = 8;
		size = 8;
	}
	else if constexpr (std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>) {
		base_alignment = 16;
		size = 16;
	}
	else if constexpr (std::is_same_v<T, glm::mat2>) {
		base_alignment = 16;
		size = 16;
	}
	else if constexpr (std::is_same_v<T, glm::mat3>) {
		base_alignment = 16;
		size = 48;
	}
	else if constexpr (std::is_same_v<T, glm::mat4>) {
		base_alignment = 16;
		size = 64;
	}
	else if constexpr (std::is_bounded_array_v<T>) { 
		auto siz = std::extent_v<T>;
		base_alignment = 16;
		size = 16 * siz;
	}
	
	return std::pair(size, base_alignment);
}
