#pragma once

#include <glad/glad.h>

#include <string>
#include <stdexcept>

#define ERROR(MESSAGE, ACTION) my_error_handler(__FILE__, __LINE__, MESSAGE, ACTION);

namespace chill_renderer {
namespace SPOOKY_FLAG {
	/* Don't ignore problems, fix them! */
	inline bool g_IGNORE_THROW = 0; 
}

enum class Error_action { 
	ignore,
	throwing,
	terminating,
	logging
};

void gl_debug_out(GLenum a_source,
	GLenum a_type,
	GLuint a_id,
	GLenum a_severity,
	GLsizei a_length,
	const GLchar* a_message,
	const void* a_userParam
	);

void my_error_handler(const std::string& file, int line, const std::string& message, Error_action action); 

class GenericException : public std::runtime_error {
public:
	GenericException(const std::string& msg) : std::runtime_error(msg) {}
};
}