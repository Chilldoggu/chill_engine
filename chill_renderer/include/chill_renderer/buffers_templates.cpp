template<typename T>
constexpr decltype(auto) to_enum_elem_type(T enumerator) noexcept {
	return static_cast<std::underlying_type_t<T>>(enumerator);
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
