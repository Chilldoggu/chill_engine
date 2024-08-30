#include <windows.h>      // For common windows data types and function headers
#include <shobjidl.h>     // IFileDialog, IShellItem

#include <string>    // wstring
#include <vector>	 // vector
#include <cstdlib> // wsctombs_s

#include "chill_engine/file_manager.hpp"
#include "chill_engine/assert.hpp" // GenericException

#define CHECK_HRESULT(expr)												       \
do {																	       \
	if (!SUCCEEDED(expr))												       \
		throw GenericException("[ERROR] Couldn't create WIN32 FileDialog.\n"); \
} while (false);

std::string wstos(std::wstring a_ws_src) {
    static constexpr int BUF_SIZ{ 1024 };
	size_t len;
    char* buffer = new char[BUF_SIZ]; 
    wcstombs_s(&len, buffer, (size_t)BUF_SIZ, a_ws_src.c_str(), (size_t)BUF_SIZ - 1);
    std::string ret{ buffer };
    delete[] buffer;
    return ret;
}

// Referenced from: https://learn.microsoft.com/en-us/windows/win32/shell/common-file-dialog?redirectedfrom=MSDN#basic-usage
std::wstring basic_file_open(std::wstring a_title, std::vector<std::pair<std::wstring, std::wstring>> a_save_types) {
	std::wstring ret_filepath{ L"" };

	COMDLG_FILTERSPEC* ptr_save_types = new COMDLG_FILTERSPEC[a_save_types.size()];
	std::vector<COMDLG_FILTERSPEC> save_types_container(a_save_types.size());
	for (size_t i = 0; i < a_save_types.size(); i++) {
		ptr_save_types[i].pszName = a_save_types[i].first.c_str();
		ptr_save_types[i].pszSpec = a_save_types[i].second.c_str();
	}


	// CoCreate the File Open Dialog object.
	IFileDialog* pfd = NULL;
	CHECK_HRESULT(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)));

	// Set the options on the dialog.
	DWORD dwFlags;
	// Get the options first to not override existing options.
	CHECK_HRESULT(pfd->GetOptions(&dwFlags));
	// Get shell items only for file system items.
	CHECK_HRESULT(pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM));

	// Set the file types to display only.
	CHECK_HRESULT(pfd->SetFileTypes((UINT)a_save_types.size(), ptr_save_types));
	CHECK_HRESULT(pfd->SetFileTypeIndex(1));
	CHECK_HRESULT(pfd->SetDefaultExtension(L"obj"));

	CHECK_HRESULT(pfd->SetTitle(L"Import model"));

	// Show the dialog. Check for possible errors.  
	if (SUCCEEDED(pfd->Show(NULL))) {
		// Obtain the result, once the user clicks the 'Open' button.
		// The result is an IShellItem object.
		IShellItem* psiResult;
		CHECK_HRESULT(pfd->GetResult(&psiResult));

		// We are just going to print out the name of the file for sample sake.
		PWSTR pszFilePath = NULL;
		CHECK_HRESULT(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath));
		ret_filepath = pszFilePath;

		CoTaskMemFree(static_cast<void*>(pszFilePath));

		psiResult->Release();
	}
	pfd->Release();
	delete[] ptr_save_types;

	return ret_filepath;
}