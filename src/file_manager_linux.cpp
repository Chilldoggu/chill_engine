#include <cstdlib> // wsctombs_s
#include <string>  // wstring
#include <vector>  // vector
#include <utility> // pair

#include "file_manager.hpp"
#include "assert.hpp" // GenericException

namespace chill_engine {
std::string wstos(const std::wstring& a_ws_src) {
	static constexpr int BUF_SIZ{ 1024 };
	size_t len;
	char* buffer = new char[BUF_SIZ];
	wcstombs_s(&len, buffer, (size_t)BUF_SIZ, a_ws_src.c_str(), (size_t)BUF_SIZ - 1);
	std::string ret{ buffer };
	delete[] buffer;
	return ret;
}

std::wstring basic_file_open(const std::wstring& a_title, const std::vector<std::pair<std::wstring, std::wstring>>& a_save_types) {
	throw GenericException("LINUX FILE DIALOG IMPLEMENTATION IS NOT READY.\n");
	return L"";
} 
}