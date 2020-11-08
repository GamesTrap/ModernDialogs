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
#include <iostream>
bool DetectPresence(const std::string& executable)
{
	std::string buffer;
	buffer.resize(MaxPathOrCMD);
	FILE* in;
	
	std::string testedString = "which " + executable + " 2>/dev/null ";
	in = popen(testedString.data(), "r");
	if((fgets(buffer.data(), buffer.size(), in) != nullptr)
		&& (buffer.find_first_of(':') == std::string::npos) && (buffer.find("no ") == std::string::npos))
	{
		pclose(in);
		return true;
	}
	else
	{
		pclose(in);
		return false;
	}
}

//-------------------------------------------------------------------------------------------------------------------//

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

//-------------------------------------------------------------------------------------------------------------------//

bool GetEnvDISPLAY()
{
	return std::getenv("DISPLAY");
}

//-------------------------------------------------------------------------------------------------------------------//

bool IsDarwin()
{
	static int32_t isDarwin = -1;
	struct utsname lUtsname;
	
	if (isDarwin < 0)
		isDarwin = !uname(&lUtsname) && std::string(lUtsname.sysname) == "Darwin";
	
	return isDarwin;
}

//-------------------------------------------------------------------------------------------------------------------//

bool GraphicMode()
{
	return (GetEnvDISPLAY() || (IsDarwin() && (!std::getenv("SSH_TTY") || GetEnvDISPLAY())));
}

//-------------------------------------------------------------------------------------------------------------------//

bool XPropPresent()
{
	static int32_t xpropPresent = -1;
	
	if (xpropPresent < 0)
		xpropPresent = DetectPresence("xprop");

	return xpropPresent && GraphicMode();
}

//-------------------------------------------------------------------------------------------------------------------//

bool ZenityPresent()
{
	static int32_t zenityPresent = -1;
	
	if (zenityPresent < 0)
		zenityPresent = DetectPresence("zenity");

	return zenityPresent && GraphicMode();
}

//-------------------------------------------------------------------------------------------------------------------//

int32_t KDialogPresent()
{
	static int32_t kdialogPresent = -1;
	std::string buffer;
	buffer.resize(MaxPathOrCMD);
	FILE* in;
	std::string desktop;
	
	if(kdialogPresent < 0)
	{
		if(ZenityPresent())
		{
			auto desktopEnv = std::getenv("XDG_SESSION_DESKTOP");
			if(!desktopEnv)
			{
				desktopEnv = std::getenv("XDG_CURRENT_DESKTOP");
				if(!desktopEnv)
				{
					desktopEnv = std::getenv("DESKTOP_SESSION");
					
					if(!desktopEnv)
					{
						kdialogPresent = 0;
						return kdialogPresent;
					}
				}
			}
			desktop = std::string(desktopEnv);
			if(desktop.empty() || ((desktop != "KDE" || desktop != "kde") && (desktop != "lxqt" || desktop != "LXQT")))
			{
				kdialogPresent = 0;
				return kdialogPresent;
			}
		}

		kdialogPresent = DetectPresence("kdialog");
		if(kdialogPresent && !std::getenv("SSH_TTY"))
		{
			in = popen("kdialog --attach 2>&1", "r");
			if(fgets(buffer.data(), buffer.size(), in) != nullptr)
			{
				if (buffer.find("Unknown") == std::string::npos)
					kdialogPresent = 2;
			}
			pclose(in);
		}
	}

	return GraphicMode() ? kdialogPresent : 0;
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
	std::string dialogString;
	if(KDialogPresent())
	{
		dialogString = "kdialog";
		if (KDialogPresent() == 2 && XPropPresent())
			dialogString += " --attach=$(xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)";
		dialogString += " --getsavefilename ";

		if (!defaultPathAndFile.empty())
		{
			if (defaultPathAndFile[0] != '/')
				dialogString += "$PWD/";
			dialogString += "\"" + defaultPathAndFile + "\"";
		}
		else
			dialogString += "$PWD/";

		if(!filterPatterns.empty())
		{
			dialogString += " \"" + filterPatterns[0].first + " (" + filterPatterns[0].second + ")\n";
			for(uint32_t i = 1; i < filterPatterns.size(); i++)
			{
				if(filterPatterns[i].second.find(';') == std::string::npos)
					dialogString += filterPatterns[i].first + " (" + filterPatterns[i].second + ")\n";
				else
				{
					std::string extensions = filterPatterns[i].second;
					std::replace(extensions.begin(), extensions.end(), ';', ' ');
					dialogString += filterPatterns[i].first + " (" + extensions + ")\n";
				}
			}
		}
		if(allFiles && filterPatterns.empty())
			dialogString += "\"All Files (*.*)\"";
		else if(allFiles && !filterPatterns.empty())
			dialogString += "All Files (*.*)\"";
		else if(!allFiles && !filterPatterns.empty())
		{
			dialogString.pop_back();
			dialogString += "\"";
		}
		if(!title.empty())
			dialogString += " --title \"" + title + "\"";
	}

	FILE* in;
	if(!(in = popen(dialogString.data(), "r")))
		return {};
	buffer.resize(MaxPathOrCMD);
	while (fgets(buffer.data(), buffer.size(), in) != nullptr){}
	pclose(in);
	if (buffer[buffer.length() - 1] == '\n')
		buffer.pop_back();
	if (buffer.empty())
		return {};
	path = buffer;
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
