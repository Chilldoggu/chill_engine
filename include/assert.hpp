#pragma once

#define ERROR(MESSAGE) my_error_handler(__FILE__, __LINE__, MESSAGE);

#include <string>
#include <iostream>
#include <stdexcept>

class GenericException : public std::runtime_error {
public:
    GenericException(std::string const& msg) : std::runtime_error(msg) {}
};

void my_error_handler(const char* file, int line, const char* message);

enum class Error_action { ignore, throwing, terminating, logging };
enum class Error_code { 
    range_error, length_error, division_by_zero, 
    size_mismatch, bad_match, init_error, file_init, 
    glsl_bad_compilation, glsl_bad_linking, glsl_bad_shader_type,
    bad_type, uninitialised, file_not_found
};

const std::string error_codes[] { 
    "range_error", "length_error", "division_by_zero", 
    "size_mismatch", "bad_match", "init_error", "file_init", 
    "glsl_bad_compilation", "glsl_bad_linking", "glsl_bad_shader_type",
    "bad_type", "uninitialised", "file_not_found"
};

constexpr Error_action default_Error_action { Error_action::throwing };

template<Error_action action = default_Error_action, class C> 
constexpr void expect(C cond, Error_code x) {
    if constexpr (action == Error_action::logging)
        if (!cond()) std::cerr << "expect() failure: " << int(x) << ' ' << error_codes[int(x)] << std::endl;
    if constexpr (action == Error_action::throwing)
        if (!cond()) throw x;
    if constexpr (action == Error_action::terminating)
        if (!cond()) std::terminate();
}
