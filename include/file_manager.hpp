#pragma once

#include <string>  // wstring
#include <vector>  // vector
#include <utility> // pair

std::string wstos(std::wstring a_ws_src);

std::wstring basic_file_open(
	std::wstring a_title,
	std::vector<std::pair<std::wstring, std::wstring>> a_save_types
); 