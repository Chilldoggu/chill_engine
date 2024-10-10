#include <string>  // wstring
#include <vector>  // vector
#include <utility> // pair
#include <filesystem>

#include "chill_renderer/file_manager.hpp"
#include "chill_renderer/assert.hpp" // GenericException

namespace chill_renderer {
// Delegate all problems with wstring to filesystem::path object.
std::string wstos(const std::wstring& a_ws_src) {
	return std::filesystem::path(a_ws_src).string();
}

std::wstring basic_file_open(const std::wstring& a_title, const std::vector<std::pair<std::wstring, std::wstring>>& a_save_types) {
	throw GenericException("LINUX FILE DIALOG IMPLEMENTATION IS NOT READY.\n");
	return L"";
} 
}
