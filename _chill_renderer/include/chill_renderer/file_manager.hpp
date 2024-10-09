#pragma once

#include <string>  // wstring
#include <vector>  // vector
#include <utility> // pair

namespace chill_renderer {
std::string wstos(const std::wstring& a_ws_src);

std::wstring basic_file_open(
	const std::wstring& a_title, const std::vector<std::pair<std::wstring,std::wstring>>& a_save_types
); 
}