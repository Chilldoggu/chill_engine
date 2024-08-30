#pragma once

#include <string>
#include <iostream>
#include <stdexcept>

#define ERROR(MESSAGE, ACTION) my_error_handler(__FILE__, __LINE__, MESSAGE, ACTION);

enum class Error_action { ignore, throwing, terminating, logging };

class GenericException : public std::runtime_error {
public:
    GenericException(std::string const& msg) : std::runtime_error(msg) {}
};

void my_error_handler(std::string file, int line, std::string message, Error_action action);
