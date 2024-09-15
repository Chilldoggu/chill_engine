#include <string>
#include <sstream>
#include <iostream>

#include "chill_engine/assert.hpp"

namespace chill_engine {
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
		terminate();
		break;
	}
} 
}