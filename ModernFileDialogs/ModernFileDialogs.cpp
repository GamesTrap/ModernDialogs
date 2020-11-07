#include "ModernFileDialogs.h"

#include <algorithm>

#ifdef _WIN32
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <conio.h>
#include <direct.h>
#define SLASH "\\"
#else
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#include <sys/utsname.h>
#include <signal.h>
#define SLASH "/"
#endif

//-------------------------------------------------------------------------------------------------------------------//

constexpr static int32_t MaxPathOrCMD = 1024;
constexpr static int32_t MaxMultipleFiles = 1024;

//-------------------------------------------------------------------------------------------------------------------//

//Windows land
#ifdef _WIN32

std::wstring GetPathWithoutFinalSlashW(const std::wstring& source)
{
	std::wstring result;
	
	if (!source.empty())
	{
		std::size_t index = source.find_last_of(L'/');
		if (index == std::wstring_view::npos)
			index = source.find_last_of(L'\\');
		if (index != std::string_view::npos)
			result = source.substr(0, index);
		else
			result = {};
	}
	else
		result = {};

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

std::wstring GetLastNameW(const std::wstring& source)
{
	std::wstring result;
	
	if (!source.empty())
	{
		std::size_t index = source.find_last_of(L'/');
		if (index == std::wstring_view::npos)
			index = source.find_last_of(L'\\');
		if (index != std::wstring_view::npos)
			result = source.substr(index + 1);
		else
			result = source;
	}
	else
		result = {};

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

std::wstring UTF8To16(const std::string& UTF8String)
{
	std::wstring result;

	const int32_t count = MultiByteToWideChar(CP_UTF8, 0, UTF8String.data(), -1, nullptr, 0);
	if (!count)
		return {};

	result.resize(count);

	if (!MultiByteToWideChar(CP_UTF8, 0, UTF8String.data(), -1, result.data(), static_cast<int32_t>(result.size())))
		return {};
	
	result.pop_back();

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string UTF16To8(const std::wstring& UTF16String)
{
	std::string result;

	const int32_t count = WideCharToMultiByte(CP_UTF8, 0, UTF16String.data(), -1, nullptr, 0, nullptr, nullptr);
	if (!count)
		return {};

	result.resize(count);

	if (!WideCharToMultiByte(CP_UTF8, 0, UTF16String.data(), -1, result.data(), static_cast<int32_t>(result.size()), nullptr, nullptr))
		return {};

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

bool DirExists(const std::string& dirPath)
{
	struct _stat info {};

	if (dirPath.empty())
		return false;
	const std::size_t dirLen = dirPath.length();
	if (!dirLen)
		return true;
	if ((dirLen == 2) && (dirPath[1] == ':'))
		return true;

	const std::wstring wStr = UTF8To16(dirPath);
	const int32_t statRet = _wstat(wStr.data(), &info);
	if (statRet != 0)
		return false;
	if (info.st_mode & S_IFDIR)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------------------------//

std::wstring SaveFileW(const std::wstring& title,
                       const std::wstring& defaultPathAndFile,
                       const std::vector<std::pair<std::wstring, std::wstring>>& filterPatterns,
                       const bool allFiles)
{
	static std::wstring buffer{};
	std::wstring defaultExtension{};
	std::wstring filterPatternsStr;
	std::wstring dirName;
	HRESULT hResult = CoInitializeEx(nullptr, 0);

	dirName = GetPathWithoutFinalSlashW(defaultPathAndFile);
	buffer = GetLastNameW(defaultPathAndFile);

	if (!filterPatterns.empty())
	{
		defaultExtension = filterPatterns[0].second;
		if (filterPatterns[0].first.empty())
		{
			filterPatternsStr += filterPatterns[0].second + L'\n';
			filterPatternsStr += filterPatterns[0].second;
		}
		else
			filterPatternsStr += filterPatterns[0].first + L"(" + filterPatterns[0].second + L")\n" + filterPatterns[0].second;
		for (uint32_t i = 1; i < filterPatterns.size(); i++)
		{
			if (filterPatterns[i].first.empty())
			{
				filterPatternsStr += L'\n' + filterPatterns[i].second;
				filterPatternsStr += L'\n' + filterPatterns[i].second;
			}
			else
				filterPatternsStr += L'\n' + filterPatterns[i].first + L"(" + filterPatterns[i].second + L")\n" + filterPatterns[i].second;
		}
		filterPatternsStr += L'\n';
		if (allFiles)
			filterPatternsStr += L"All Files\n*.*\n";
		else
			filterPatternsStr += L'\n';
	}
	else
	{
		defaultExtension = L"*.*";

		filterPatternsStr = L"All Files\n*.*\n";
	}

	std::replace(filterPatternsStr.begin(), filterPatternsStr.end(), L'\n', L'\0');

	OPENFILENAMEW ofn;
	std::memset(&ofn, 0, sizeof(OPENFILENAMEW));

	buffer.resize(MaxPathOrCMD);
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = GetForegroundWindow();
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = filterPatternsStr.empty() ? nullptr : filterPatternsStr.data();
	ofn.lpstrCustomFilter = nullptr;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = buffer.data();

	ofn.nMaxFile = MaxPathOrCMD;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = MaxPathOrCMD / 2;
	ofn.lpstrInitialDir = dirName.empty() ? nullptr : dirName.data();
	ofn.lpstrTitle = title.empty() ? nullptr : title.data();
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = defaultExtension.data();
	ofn.lCustData = 0L;
	ofn.lpfnHook = nullptr;
	ofn.lpTemplateName = nullptr;

	std::wstring retVal;
	if (GetSaveFileNameW(&ofn) == 0)
		retVal = {};
	else
		retVal = buffer;

	if (hResult == S_OK || hResult == S_FALSE)
		CoUninitialize();

	return retVal;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string SaveFileWinGUI(const std::string& title,
                           const std::string& defaultPathAndFile,
                           const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                           const bool allFiles)
{
	std::vector<std::pair<std::wstring, std::wstring>> wFilterPatterns(filterPatterns.size());
	for (uint32_t i = 0; i < wFilterPatterns.size(); i++)
		wFilterPatterns[i] = { UTF8To16(filterPatterns[i].first), UTF8To16(filterPatterns[i].second) };
	const std::wstring wTitle = UTF8To16(title);
	const std::wstring wDefaultPathAndFile = UTF8To16(defaultPathAndFile);

	const std::wstring wPath = SaveFileW(wTitle, wDefaultPathAndFile, wFilterPatterns, allFiles);

	if (wPath.empty())
		return {};

	std::string path = UTF16To8(wPath);
	UTF16To8({});
	
	return path;
}

//-------------------------------------------------------------------------------------------------------------------//

//Linux land
#else

bool DirExists(const std::string& dirPath)
{
	DIR* dir;
	
	if (dirPath.empty())
		return false;
	
	dir = opendir(dirPath.data());
	
	if (!dir)
		return false;
	
	closedir(dir);
	return true;
}

#endif

//-------------------------------------------------------------------------------------------------------------------//

std::string GetPathWithoutFinalSlash(const std::string& source)
{
	std::string result;

	if (!source.empty())
	{
		std::size_t index = source.find_last_of('/');
		if (index == std::string_view::npos)
			index = source.find_last_of('\\');
		if (index != std::string_view::npos)
			result = source.substr(0, index);
		else
			return {};
	}
	else
		return {};

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string GetLastName(const std::string& source)
{
	std::string result;
	
	if (!source.empty())
	{
		std::size_t index = source.find_last_of('/');
		if (index == std::string_view::npos)
			index = source.find_last_of('\\');
		if (index != std::string_view::npos)
			result = source.substr(index + 1);
		else
			return source;
	}
	else
		return {};

	return result;
}

//-------------------------------------------------------------------------------------------------------------------//

bool FilenameValid(const std::string_view filenameWithoutPath)
{
	if (filenameWithoutPath.empty())
		return false;

	for (const char& c : filenameWithoutPath)
	{
		if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' ||
			c == '\"' || c == '<' || c == '>' || c == '|')
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------------------------//

bool QuoteDetected(const std::string_view str)
{
	if (str.empty())
		return false;

	if (str.find_first_of('\'') != std::string_view::npos || str.find_first_of('\"') != std::string_view::npos)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string SaveFile(const std::string& title,
                     const std::string& defaultPathAndFile,
                     const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                     const bool allFiles)
{
	static std::string buffer{};

	if (QuoteDetected(title))
		return SaveFile("INVALID TITLE WITH QUOTES", defaultPathAndFile, filterPatterns, allFiles);
	if (QuoteDetected(defaultPathAndFile))
		return SaveFile(title, "INVALID DEFAULT_PATH WITH QUOTES", filterPatterns, allFiles);
	for(const auto& filterPattern : filterPatterns)
	{
		if (QuoteDetected(filterPattern.first) || QuoteDetected(filterPattern.second))
			return SaveFile("INVALID FILTER_PATTERN WITH QUOTES", defaultPathAndFile, {}, allFiles);
	}

	std::string path;
#ifdef _WIN32
	path = SaveFileWinGUI(title, defaultPathAndFile, filterPatterns, allFiles);
	buffer = path;
#else
	//Linux TODO
#endif

	if (path.empty())
		return {};
	std::string str = GetPathWithoutFinalSlash(path);
	if (str.empty() && !DirExists(str))
		return {};
	str = GetLastName(path);
	if (!FilenameValid(str))
		return {};
	
	return path;
}
