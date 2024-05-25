#include "assert.hpp"

void my_error_handler(const char* file, int line, const char* message) {
    std::cerr << "[ERROR] " << file << ':' << line << ' ' << message << std::endl;
}
