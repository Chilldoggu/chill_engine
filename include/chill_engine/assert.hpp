#pragma once

#include <string>
#include <stdexcept>

#define ERROR(MESSAGE, ACTION) my_error_handler(__FILE__, __LINE__, MESSAGE, ACTION);

namespace chill_engine {
enum class Error_action { ignore, throwing, terminating, logging };

class GenericException : public std::runtime_error {
public:
	GenericException(const std::string& msg) : std::runtime_error(msg) {}
};

void my_error_handler(const std::string& file, int line, const std::string& message, Error_action action); 
}