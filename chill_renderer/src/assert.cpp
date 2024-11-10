#include <string>
#include <sstream>
#include <iostream>
#include <format>

#include "chill_renderer/assert.hpp"

namespace chill_renderer {
void gl_debug_out(GLenum a_source, GLenum a_type, GLuint a_id, GLenum a_severity,
                  GLsizei a_length, const GLchar* a_message, const void* a_userParam)
{
	// Ignoring not significant error/warning codes.
	if (SPOOKY_FLAG::g_IGNORE_THROW || a_id == 131169 || a_id == 131185) return; 

	bool throw_flag = false;

	std::stringstream full_msg{};
	full_msg << "[GL_DEBUG_OUT]\n";
	full_msg << std::format("\t[MESSAGE] ID {}, {}", a_id, a_message) << '\n';

	std::string src{};
	switch (a_source) {
	case GL_DEBUG_SOURCE_API:             src = "API"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     src = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     src = "Application"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   src = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_OTHER:           src = "Other"; break;
	}
	full_msg << std::format("\t[SOURCE] {}", src) << '\n';

	std::string err_type{};
	switch (a_type) {
	case GL_DEBUG_TYPE_ERROR:               err_type = "Error"; break;
	case GL_DEBUG_TYPE_MARKER:              err_type = "Marker"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           err_type = "Pop Group"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          err_type = "Push Group"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         err_type = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         err_type = "Performance"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  err_type = "Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: err_type = "Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_OTHER:               err_type = "Other"; break;
	}
	full_msg << std::format("\t[ERROR_TYPE] {}", err_type) << '\n';

	std::string severity{};
	switch (a_severity) {
	case GL_DEBUG_SEVERITY_HIGH: severity = "High"; throw_flag = true; break;
	case GL_DEBUG_SEVERITY_LOW: severity = "Low"; throw_flag = true; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severity = "Medium"; throw_flag = true; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severity = "Notification"; break;
	}
	full_msg << std::format("\t[SEVERITY] {}", severity) << '\n';

	std::cerr << full_msg.str();
	if (throw_flag) {
		throw GenericException("[GL_DEBUG_OUT] Generating exception...");
	}
}

void my_error_handler(const std::string& file, int line, const std::string& message, Error_action action) {
	std::stringstream err_msg;
	err_msg << "[ERROR] [" << file << ":" << line << "] " << message << "\n";

	switch (action) {
	case Error_action::ignore:
		return;
	case Error_action::logging:
		std::cerr << err_msg.str() << std::flush;
		break;
	case Error_action::throwing:
		std::cerr << err_msg.str() << std::flush;
		throw GenericException(err_msg.str());
		break;
	case Error_action::terminating:
		std::terminate();
		break;
	}
} 
}
