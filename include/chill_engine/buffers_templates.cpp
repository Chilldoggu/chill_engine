#include "buffers.hpp"
#include <type_traits>

class Application;
enum class ResourceType;

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

template<typename U, typename... T>
UniformBuffer<U, T...>::UniformBuffer(const std::vector<std::string>& a_uniform_names) {
	glGenBuffers(1, &m_id);
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
	push_elements<U, T...>(a_uniform_names);
	create_buffer(); 
}

template<typename U, typename... T>
UniformBuffer<U, T...>::UniformBuffer(const UniformBuffer& a_uni_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, a_uni_buf.m_id);

	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point; 
}

template<typename U, typename... T>
UniformBuffer<U, T...>::UniformBuffer(UniformBuffer&& a_uni_buf) noexcept {
	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point;

	a_uni_buf.m_id = EMPTY_VBO; 
}

template<typename U, typename... T>
UniformBuffer<U, T...>::~UniformBuffer() {
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::UNIFORM_BUFFERS, m_id)) {
			glDeleteFramebuffers(1, &m_id);
		}
	}
}

template<typename U, typename... T>
UniformBuffer<U, T...>& UniformBuffer<U, T...>::operator=(const UniformBuffer& a_uni_buf) {
	Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, a_uni_buf.m_id);

	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point;

	return *this; 
}

template<typename U, typename... T>
UniformBuffer<U, T...>& UniformBuffer<U, T...>::operator=(UniformBuffer&& a_uni_buf) noexcept {
	m_id = a_uni_buf.m_id;
	m_size = a_uni_buf.m_size;
	m_elements = a_uni_buf.m_elements;
	m_binding_point = a_uni_buf.m_binding_point;

	a_uni_buf.m_id = EMPTY_VBO;

	return *this; 
}

template<typename U, typename... T>
UniformBufferElement& UniformBuffer<U, T...>::operator[](const std::string& a_uniform_name) {
	auto it = m_elements.find(a_uniform_name);
	if (it == m_elements.end())
		ERROR(std::format("[UNIFORMBUFFER::OPERATOR[]] Uniform name \"{}\" is not present in this uniform buffer object.", a_uniform_name), Error_action::throwing);
	return (*it).second; 
}

template<typename U, typename... T>
template<typename S>
void UniformBuffer<U, T...>::push_element(const std::string& a_uniform_name) {
	if (m_id == EMPTY_VBO) {
		glGenBuffers(1, &m_id);
		Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
	}

	auto [size, base_alignment] = get_size_and_base_alignment<S>();

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
template<typename S, typename... R>
void UniformBuffer<U, T...>::push_elements(const std::vector<std::string>& a_uniform_names) {
	if constexpr (std::is_same_v<S, EmptyType>)
		return;

	push_element<S>(a_uniform_names.at(0));
	push_elements<R...>(std::vector(a_uniform_names.begin() + 1, a_uniform_names.end())); 
}

template<typename U, typename... T>
bool UniformBuffer<U, T...>::check_status() const {
	if (m_id == EMPTY_VBO || m_binding_point == -1)
		return false;
	return true; 
}

template<typename U, typename... T>
void UniformBuffer<U, T...>::create_buffer() {
	if (m_id == EMPTY_VBO) {
		glGenBuffers(1, &m_id);
		Application::get_instance().get_rmanager().inc_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_id);
	glBufferData(GL_UNIFORM_BUFFER, m_size, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0); 
}

template<typename U, typename... T>
void UniformBuffer<U, T...>::set_binding_point(int a_binding_point) {
	m_binding_point = a_binding_point;
	glBindBufferBase(GL_UNIFORM_BUFFER, a_binding_point, m_id); 
}

template<typename U, typename... T>
void UniformBuffer<U, T...>::clear() {
	m_elements.clear();
	if (m_id != EMPTY_VBO) {
		Application::get_instance().get_rmanager().dec_ref_count(ResourceType::UNIFORM_BUFFERS, m_id);
		if (!Application::get_instance().get_rmanager().chk_ref_count(ResourceType::UNIFORM_BUFFERS, m_id)) {
			glDeleteFramebuffers(1, &m_id);
		}
		m_id = EMPTY_VBO;
	} 
}

template<typename U, typename... T>
auto UniformBuffer<U, T...>::get_elements() const {
	return m_elements; 
}

template<typename U, typename... T>
template<typename S>
auto UniformBuffer<U, T...>::get_size_and_base_alignment() {
	int base_alignment{};
	int size{}; 

	if constexpr (std::is_same_v<S, float> || std::is_same_v<S, int> || std::is_same_v<S, bool>) {
		base_alignment = 4;
		size = 4;
	}
	else if constexpr (std::is_same_v<S, glm::vec1> || std::is_same_v<S, glm::vec2>) {
		base_alignment = 8;
		size = 8;
	}
	else if constexpr (std::is_same_v<S, glm::vec3> || std::is_same_v<S, glm::vec4>) {
		base_alignment = 16;
		size = 16;
	}
	else if constexpr (std::is_same_v<S, glm::mat2>) {
		base_alignment = 16;
		size = 16;
	}
	else if constexpr (std::is_same_v<S, glm::mat3>) {
		base_alignment = 16;
		size = 48;
	}
	else if constexpr (std::is_same_v<S, glm::mat4>) {
		base_alignment = 16;
		size = 64;
	}
	else if constexpr (std::is_bounded_array_v<S>) { 
		auto siz = std::extent_v<S>;
		base_alignment = 16;
		size = 16 * siz;
	}
	
	return std::pair(size, base_alignment);
}
