#include <string>

#include "assert.hpp" 

void my_error_handler(std::string file, int line, std::string message, Error_action action) {
	std::string err_msg = "[ERROR] [" + file + ":" + std::to_string(line) + "] " + message + "\n";

    switch (action) {
    case Error_action::ignore :
        return;
    case Error_action::logging :
		std::cerr << err_msg << std::flush;
        break;
    case Error_action::throwing :
		std::cerr << err_msg << std::flush;
        throw GenericException(err_msg);
        break;
    case Error_action::terminating:
        terminate();
        break;
    }
}
