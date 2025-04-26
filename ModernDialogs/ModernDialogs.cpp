/*
MIT License

Copyright (c) 2020 - 2025 Jan "GamesTrap" Schürkamp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ModernDialogs.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <array>

#ifdef _WIN32
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <conio.h>
#include <direct.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <termios.h>
#include <sys/utsname.h>
#include <signal.h>
#endif

#if _MSVC_LANG >= 202002L || __cplusplus >= 202002L
#define CPP20Constexpr constexpr
#else
#define CPP20Constexpr
#endif

namespace
{
	//-------------------------------------------------------------------------------------------------------------------//

	constexpr static int32_t MaxPathOrCMD = 1024; //TODO Get rid of this limit
	constexpr static int32_t MaxMultipleFiles = 1024; //TODO Get rid of this limit

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::string GetPathWithoutFinalSlash(const std::string& source)
	{
		if(source.empty())
			return "";

		std::size_t index = source.find_last_of('/');
		if (index == std::string_view::npos)
			index = source.find_last_of('\\');

		if (index == std::string_view::npos)
			return "";

		return source.substr(0, index);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::string GetLastName(const std::string& source)
	{
		if (source.empty())
			return "";

		std::size_t index = source.find_last_of('/');
		if (index == std::string_view::npos)
			index = source.find_last_of('\\');

		if (index == std::string_view::npos)
			return source;

		return source.substr(index + 1);
	}

	//-------------------------------------------------------------------------------------------------------------------//

//Windows land
#ifdef _WIN32

	[[nodiscard]] CPP20Constexpr std::wstring GetPathWithoutFinalSlashW(const std::wstring& source)
	{
		if(source.empty())
			return L"";

		std::size_t index = source.find_last_of(L'/');
		if (index == std::wstring_view::npos)
			index = source.find_last_of(L'\\');

		if (index == std::wstring_view::npos)
			return L"";

		return source.substr(0, index);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::wstring GetLastNameW(const std::wstring& source)
	{
		if (source.empty())
			return L"";

		std::size_t index = source.find_last_of(L'/');
		if (index == std::wstring_view::npos)
			index = source.find_last_of(L'\\');

		if (index == std::wstring_view::npos)
			return source;

		return source.substr(index + 1);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::wstring UTF8To16(const std::string& UTF8String)
	{
		const int32_t count = MultiByteToWideChar(CP_UTF8, 0, UTF8String.data(), -1, nullptr, 0);
		if (!count)
			return L"";

		std::wstring result(count, L'\0');

		if (!MultiByteToWideChar(CP_UTF8, 0, UTF8String.data(), -1, result.data(), static_cast<int32_t>(result.size())))
			return L"";

		result.pop_back(); //Remove the extra null terminator

		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string UTF16To8(const std::wstring& UTF16String)
	{
		const int32_t count = WideCharToMultiByte(CP_UTF8, 0, UTF16String.data(), -1, nullptr, 0, nullptr, nullptr);
		if (!count)
			return "";

		std::string result(count, '\0');

		if (!WideCharToMultiByte(CP_UTF8, 0, UTF16String.data(), -1, result.data(), static_cast<int32_t>(result.size()), nullptr, nullptr))
			return "";

		result.pop_back(); //Remove the extra null terminator

		return result;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] BOOL CALLBACK BrowseCallbackProcWEnum(HWND hwndChild, LPARAM)
	{
		std::wstring buffer(255, L'\0');
		GetClassNameW(hwndChild, buffer.data(), static_cast<int32_t>(buffer.size()));
		if(buffer == L"SysTreeView32")
		{
			HTREEITEM const hNode = TreeView_GetSelection(hwndChild);
			TreeView_EnsureVisible(hwndChild, hNode);
			return FALSE;
		}

		return TRUE;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	int32_t CALLBACK BrowseCallbackProcW(HWND hwnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData)
	{
		switch(uMsg)
		{
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTIONW, TRUE, pData);
			break;

		case BFFM_SELCHANGED:
			EnumChildWindows(hwnd, BrowseCallbackProcWEnum, 0);
			break;

		default:
			break;
		}

		return 0;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::wstring SaveFileW(const std::wstring& title,
										const std::wstring& defaultPathAndFile,
										const std::vector<std::pair<std::wstring, std::wstring>>& filterPatterns,
										const bool allFiles)
	{
		std::wstring defaultExtension = L"*.*";
		std::wstring filterPatternsStr = L"All Files\n*.*\n";
		HRESULT hResult = CoInitializeEx(nullptr, 0);

		const std::wstring dirName = GetPathWithoutFinalSlashW(defaultPathAndFile);
		std::wstring buffer = GetLastNameW(defaultPathAndFile);

		if (!filterPatterns.empty())
		{
			const auto& [frontName, frontExtensions] = filterPatterns.front();

			defaultExtension = frontExtensions;
			filterPatternsStr.clear();

			if (frontName.empty())
			{
				filterPatternsStr += frontExtensions + L'\n';
				filterPatternsStr += frontExtensions;
			}
			else
				filterPatternsStr += frontName + L"(" + frontExtensions + L")\n" + frontExtensions;

			for (uint32_t i = 1u; i < filterPatterns.size(); ++i)
			{
				const auto& [name, extensions] = filterPatterns[i];

				if (name.empty())
				{
					filterPatternsStr += L'\n' + extensions;
					filterPatternsStr += L'\n' + extensions;
				}
				else
					filterPatternsStr += L'\n' + name + L"(" + extensions + L")\n" + extensions;
			}
			filterPatternsStr += L'\n';

			if (allFiles)
				filterPatternsStr += L"All Files\n*.*\n";
			else
				filterPatternsStr += L'\n';
		}

		std::replace(filterPatternsStr.begin(), filterPatternsStr.end(), L'\n', L'\0');

		OPENFILENAMEW ofn{};

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
			retVal = L"";
		else
			retVal = buffer;

		if (hResult == S_OK || hResult == S_FALSE)
			CoUninitialize();

		return retVal;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string SaveFileWinGUI(const std::string& title,
											const std::string& defaultPathAndFile,
											const std::vector<std::pair<std::string, std::string>>& filterPatterns,
											const bool allFiles)
	{
		std::vector<std::pair<std::wstring, std::wstring>> wFilterPatterns(filterPatterns.size());
		for (uint32_t i = 0; i < wFilterPatterns.size(); i++)
		{
			const auto& [name, extensions] = filterPatterns[i];
			wFilterPatterns[i] = { UTF8To16(name), UTF8To16(extensions) };
		}

		const std::wstring wTitle = UTF8To16(title);
		const std::wstring wDefaultPathAndFile = UTF8To16(defaultPathAndFile);

		const std::wstring wPath = SaveFileW(wTitle, wDefaultPathAndFile, wFilterPatterns, allFiles);

		if (wPath.empty())
			return "";

		return UTF16To8(wPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::vector<std::wstring> OpenFileW(const std::wstring& title,
													const std::wstring& defaultPathAndFile,
													const std::vector<std::pair<std::wstring, std::wstring>>& filterPatterns,
													const bool allowMultipleSelects,
													const bool allFiles)
	{
		HRESULT hResult = CoInitializeEx(nullptr, 0);

		const std::wstring dirName = GetPathWithoutFinalSlashW(defaultPathAndFile);
		std::wstring buffer = GetLastNameW(defaultPathAndFile);

		if (allowMultipleSelects)
			buffer.resize(MaxMultipleFiles * MaxPathOrCMD + 1);
		else
			buffer.resize(MaxPathOrCMD + 1);

		std::wstring filterPatternsStr = L"All Files\n*.*\n";
		if (!filterPatterns.empty())
		{
			filterPatternsStr.clear();

			const auto& [firstName, firstExtensions] = filterPatterns[0];

			if (firstName.empty())
			{
				filterPatternsStr += firstExtensions + L'\n';
				filterPatternsStr += firstExtensions;
			}
			else
				filterPatternsStr += firstName + L"(" + firstExtensions + L")\n" + firstExtensions;

			for (uint32_t i = 1; i < filterPatterns.size(); ++i)
			{
				const auto& [name, extensions] = filterPatterns[i];

				if (name.empty())
				{
					filterPatternsStr += L'\n' + extensions;
					filterPatternsStr += L'\n' + extensions;
				}
				else
					filterPatternsStr += L'\n' + name + L"(" + extensions + L")\n" + extensions;
			}
			filterPatternsStr += L'\n';
			if (allFiles)
				filterPatternsStr += L"All Files\n*.*\n";
			else
				filterPatternsStr += L'\n';
		}

		std::replace(filterPatternsStr.begin(), filterPatternsStr.end(), L'\n', L'\0');

		OPENFILENAMEW ofn{};
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = GetForegroundWindow();
		ofn.hInstance = nullptr;
		ofn.lpstrFilter = filterPatternsStr.empty() ? nullptr : filterPatternsStr.data();
		ofn.lpstrCustomFilter = nullptr;
		ofn.nMaxCustFilter = 0;
		ofn.nFilterIndex = 1;
		ofn.lpstrFile = buffer.data();

		ofn.nMaxFile = static_cast<uint32_t>(buffer.size());
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = MaxPathOrCMD / 2;
		ofn.lpstrInitialDir = dirName.empty() ? nullptr : dirName.data();
		ofn.lpstrTitle = title.empty() ? nullptr : title.data();
		ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.nFileOffset = 0;
		ofn.nFileExtension = 0;
		ofn.lpstrDefExt = nullptr;
		ofn.lCustData = 0L;
		ofn.lpfnHook = nullptr;
		ofn.lpTemplateName = nullptr;
		if (allowMultipleSelects)
			ofn.Flags |= OFN_ALLOWMULTISELECT;

		std::vector<std::wstring> paths{};
		if (!GetOpenFileNameW(&ofn))
			buffer = L"";
		else
		{
			if (allowMultipleSelects)
			{
				std::wstring folder{};
				do
				{
					const auto x = buffer.find_first_of(L'\0');
					const std::wstring s = buffer.substr(0, x);

					if (folder.empty())
						folder = s + L"\\";
					else
						paths.push_back(folder + s);

					buffer = buffer.substr(x + 1, buffer.size() - (x + 1));
				} while (buffer[0] != L'\0');
			}
			else
				paths.push_back(buffer);
		}

		if (hResult == S_OK || hResult == S_FALSE)
			CoUninitialize();

		return paths;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::vector<std::string> OpenFileWinGUI(const std::string& title,
														const std::string& defaultPathAndFile,
														const std::vector<std::pair<std::string, std::string>>& filterPatterns,
														const bool allowMultipleSelects,
														const bool allFiles)
	{
		std::vector<std::pair<std::wstring, std::wstring>> wFilterPatterns(filterPatterns.size());
		for(uint32_t i = 0; i < wFilterPatterns.size(); ++i)
		{
			const auto& [name, extensions] = filterPatterns[i];
			wFilterPatterns[i] = { UTF8To16(name), UTF8To16(extensions) };
		}

		std::wstring wTitle{};
		if (!title.empty())
			wTitle = UTF8To16(title);

		std::wstring wDefaultPathAndFile{};
		if (!defaultPathAndFile.empty())
			wDefaultPathAndFile = UTF8To16(defaultPathAndFile);

		const std::vector<std::wstring> wPaths = OpenFileW(wTitle, wDefaultPathAndFile, wFilterPatterns, allowMultipleSelects, allFiles);
		if (wPaths.empty())
			return {};

		std::vector<std::string> paths(wPaths.size());
		for (uint32_t i = 0; i < paths.size(); ++i)
			paths[i] = UTF16To8(wPaths[i]);

		return paths;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::wstring SelectFolderW(const std::wstring& title, const std::wstring& defaultPath)
	{
		std::wstring buffer(MaxPathOrCMD, L'\0');

		HRESULT hResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

		BROWSEINFOW info{};

		info.hwndOwner = GetForegroundWindow();
		info.pidlRoot = nullptr;
		info.pszDisplayName = buffer.data();
		info.lpszTitle = title.empty() ? nullptr : title.data();
		if (hResult == S_OK || hResult == S_FALSE)
			info.ulFlags = BIF_USENEWUI;
		info.lpfn = BrowseCallbackProcW;
		info.lParam = reinterpret_cast<LPARAM>(defaultPath.data());
		info.iImage = -1;

		std::wstring retVal{};
		LPITEMIDLIST lpItem = SHBrowseForFolderW(&info);
		if(lpItem)
		{
			SHGetPathFromIDListW(lpItem, buffer.data());
			retVal = buffer;
		}

		if (hResult == S_OK || hResult == S_FALSE)
			CoUninitialize();

		return retVal;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string SelectFolderWinGUI(const std::string& title, const std::string& defaultPath)
	{
		std::wstring wTitle{};
		if (!title.empty())
			wTitle = UTF8To16(title);

		std::wstring wDefaultPath{};
		if (!defaultPath.empty())
			wDefaultPath = UTF8To16(defaultPath);

		const std::wstring wPath = SelectFolderW(wTitle, wDefaultPath);

		if (wPath.empty())
			return {};

		return UTF16To8(wPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] constexpr MD::Selection GetSelection(const int32_t response, const MD::Buttons buttons)
	{
		switch(response)
		{
		case IDOK:
			return buttons == MD::Buttons::Quit ? MD::Selection::Quit : MD::Selection::OK;

		case IDCANCEL:
			return MD::Selection::Cancel;

		case IDYES:
			return MD::Selection::Yes;

		case IDNO:
			return MD::Selection::No;

		default:
			return MD::Selection::None;
		}
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] constexpr uint32_t GetIcon(const MD::Style style)
	{
		switch(style)
		{
		case MD::Style::Info:
			return MB_ICONINFORMATION;

		case MD::Style::Warning:
			return MB_ICONWARNING;

		case MD::Style::Error:
			return MB_ICONERROR;

		case MD::Style::Question:
			return MB_ICONQUESTION;

		default:
			return MB_ICONINFORMATION;
		}
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] constexpr uint32_t GetButtons(const MD::Buttons buttons)
	{
		switch(buttons)
		{
		case MD::Buttons::OK:
		case MD::Buttons::Quit: //There's no 'Quit' button on Windows so use OK
			return MB_OK;

		case MD::Buttons::OKCancel:
			return MB_OKCANCEL;

		case MD::Buttons::YesNo:
			return MB_YESNO;

		default:
			return MB_OK;
		}
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] MD::Selection ShowMsgBoxWinGUI(const std::string& title,
												const std::string& message,
												const MD::Style style,
												const MD::Buttons buttons)
	{
		std::wstring wTitle{};
		if (!title.empty())
			wTitle = UTF8To16(title);

		std::wstring wMessage{};
		if (!message.empty())
			wMessage = UTF8To16(message);

		uint32_t flags = MB_TASKMODAL;

		flags |= GetIcon(style);
		flags |= GetButtons(buttons);

		return GetSelection(MessageBoxW(nullptr, wMessage.c_str(), wTitle.c_str(), flags), buttons);
	}

	//-------------------------------------------------------------------------------------------------------------------//

//Linux land
#else

	std::string Python3Name{};

	[[nodiscard]] bool DetectPresence(const std::string& executable)
	{
		std::array<char, 128> buffer{};
		std::string output{};

		const std::string cmd = "which " + executable + " 2>/dev/null ";

		FILE* const in = popen(cmd.c_str(), "r");

		while(fgets(buffer.data(), buffer.size(), in) != nullptr)
			output += buffer.data();

		pclose(in);

		return output.find_first_of(':') == std::string::npos && output.find("no ") == std::string::npos;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool GetEnvDISPLAY()
	{
		return std::getenv("DISPLAY");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool IsDarwin()
	{
		static int32_t isDarwin = -1;
		struct utsname lUtsname;

		if (isDarwin < 0)
			isDarwin = !uname(&lUtsname) && std::string_view(lUtsname.sysname) == "Darwin";

		return isDarwin;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool GraphicMode()
	{
		return (GetEnvDISPLAY() || (IsDarwin() && GetEnvDISPLAY()));
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool XPropPresent()
	{
		static int32_t xpropPresent = -1;

		if (xpropPresent < 0)
			xpropPresent = DetectPresence("xprop");

		return xpropPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool ZenityPresent()
	{
		static int32_t zenityPresent = -1;

		if (zenityPresent < 0)
			zenityPresent = DetectPresence("zenity");

		return zenityPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] int32_t Zenity3Present()
	{
		static int32_t zenity3Present = -1;

		if(zenity3Present < 0)
		{
			zenity3Present = 0;
			if(ZenityPresent())
			{
				std::array<char, 128> buffer{};
				std::string output{};
				FILE* const in = popen("zenity --version", "r");

				while(fgets(buffer.data(), buffer.size(), in) != nullptr)
					output += buffer.data();

				if(std::stoi(output) >= 3)
				{
					zenity3Present = 3;
					const int32_t temp = std::stoi(output.substr(output.find_first_not_of('.') + 2));
					if(temp >= 18)
						zenity3Present = 5;
					else if(temp >= 10)
						zenity3Present = 4;
				}
				else if((std::stoi(output) == 2) && (std::stoi(output.substr(output.find_first_not_of('.') + 2)) >= 32))
					zenity3Present = 2;

				pclose(in);
			}
		}

		return GraphicMode() ? zenity3Present : 0;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool MateDialogPresent()
	{
		static int32_t matedialogPresent = -1;

		if(matedialogPresent < 0)
			matedialogPresent = DetectPresence("matedialog");

		return matedialogPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool ShellementaryPresent()
	{
		static int32_t shellementaryPresent = -1;

		if(shellementaryPresent < 0)
			shellementaryPresent = DetectPresence("shellementary");

		return shellementaryPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool QarmaPresent()
	{
		static int32_t qarmaPresent = -1;

		if(qarmaPresent < 0)
			qarmaPresent = DetectPresence("qarma");

		return qarmaPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool YadPresent()
	{
		static int32_t yadPresent = -1;

		if(yadPresent < 0)
			yadPresent = DetectPresence("yad");

		return yadPresent && GraphicMode();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool Python3Present()
	{
		static int32_t python3Present = -1;

		if(python3Present < 0)
		{
			python3Present = 0;
			Python3Name = "python3";
			if(DetectPresence(Python3Name))
				python3Present = 1;
			else
			{
				for(int32_t i = 10; i >= 0; --i)
				{
					Python3Name = "python3." + std::to_string(i);
					if(DetectPresence(Python3Name))
					{
						python3Present = 1;
						break;
					}
				}
			}
		}

		return python3Present;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool TryCommand(const std::string& command)
	{
		std::array<char, 128> buffer{};
		std::string output{};

		FILE* const in = popen(command.data(), "r");
		while(fgets(buffer.data(), buffer.size(), in) != nullptr)
			output += buffer.data();

		pclose(in);

		return !output.empty();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool TKinter3Present()
	{
		static int32_t tkinter3Present = -1;

		if(tkinter3Present < 0)
		{
			tkinter3Present = 0;
			if(Python3Present())
			{
				static const std::string pythonParams = "-S -c \"try:\n\timport tkinter;\nexcept:\n\tprint(0);\"";
				const std::string pythonCommand = Python3Name + " " + pythonParams;
				tkinter3Present = TryCommand(pythonCommand);
			}
		}

		return tkinter3Present && GraphicMode() && !IsDarwin();
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] int32_t KDialogPresent()
	{
		static int32_t kdialogPresent = -1;

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
				const std::string desktop(desktopEnv);
				if(desktop.empty() || (desktop != "KDE" && desktop != "kde" && desktop != "lxqt" && desktop != "LXQT"))
				{
					kdialogPresent = 0;
					return kdialogPresent;
				}
			}

			kdialogPresent = DetectPresence("kdialog");
			if(kdialogPresent)
			{
				std::array<char, 128> buffer{};
				std::string output{};

				FILE* const in = popen("kdialog --attach 2>&1", "r");
				while(fgets(buffer.data(), buffer.size(), in) != nullptr)
					output += buffer.data();

				pclose(in);

				if (output.find("Unknown") == std::string::npos)
					kdialogPresent = 2;
			}
		}

		return GraphicMode() ? kdialogPresent : 0;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	constexpr std::string_view XPropCmd = " --attach=$(sleep .01;xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)";

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::string GetKDialogFileCommandFilterPart(const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                                     const bool allFiles)
	{
		std::string dialogString{};

		if(!filterPatterns.empty())
		{
			dialogString += " \"";
			for(const auto& [name, extensions] : filterPatterns)
			{
				if(extensions.find(';') == std::string::npos)
					dialogString += name + " (" + extensions + ")\n";
				else
				{
					std::string exts = extensions;
					std::replace(exts.begin(), exts.end(), ';', ' ');
					dialogString += name + " (" + exts + ")\n";
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

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetKDialogBaseFileCommand(const std::string& title,
		                                                const std::string& defaultPathAndFile,
		                                                const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                const bool allFiles,
													    const std::string& commandAction)
	{
		std::string dialogString = "kdialog";

		if (KDialogPresent() == 2 && XPropPresent())
			dialogString += XPropCmd;

		dialogString += " " + commandAction + " ";

		if (!defaultPathAndFile.empty())
		{
			if (defaultPathAndFile[0] != '/')
				dialogString += "$PWD/";
			dialogString += "\"" + defaultPathAndFile + "\"";
		}
		else
			dialogString += "$PWD/";

		dialogString += GetKDialogFileCommandFilterPart(filterPatterns, allFiles);

		if(!title.empty())
			dialogString += " --title \"" + title + "\"";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetKDialogSaveFileCommand(const std::string& title,
		                                                const std::string& defaultPathAndFile,
		                                                const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                const bool allFiles)
	{
		return GetKDialogBaseFileCommand(title, defaultPathAndFile, filterPatterns, allFiles, "--getsavefilename");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetKDialogOpenFileCommand(const std::string& title,
		                                                const std::string& defaultPathAndFile,
		                                                const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                const bool allowMultipleSelects,
		                                                const bool allFiles)
	{
		std::string dialogAction = "--getopenfilename";
		if(allowMultipleSelects)
			dialogAction += " --multiple --separate-output";

		return GetKDialogBaseFileCommand(title, defaultPathAndFile, filterPatterns, allFiles, dialogAction);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::string GetGenericFileCommandFilterPart(const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                                     const bool allFiles)
	{
		std::string dialogString{};

		if(!filterPatterns.empty())
		{
			for(const auto& [name, extensions] : filterPatterns)
			{
				if(extensions.find(';') == std::string::npos)
					dialogString += " --file-filter='" + name + " | " + extensions + "'";
				else
				{
					std::string exts = extensions;
					std::size_t index = 0;
					while((index = exts.find(';')) != std::string::npos)
						exts.replace(index, 1, " | ");
					dialogString += " --file-filter='" + name + " | " + exts + "'";
				}
			}
		}

		if(allFiles)
			dialogString += " --file-filter='All Files | *'";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetYadBaseFileCommand(const std::string& title,
		                                            const std::string& defaultPathAndFile,
		                                            const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                            const bool allFiles,
													const std::string& commandAction)
	{
		std::string dialogString = "yad " + commandAction;

		if(!title.empty())
			dialogString += " --title=\"" + title + "\"";

		if(!defaultPathAndFile.empty())
			dialogString += " --filename=\"" + defaultPathAndFile + "\"";

		dialogString += GetGenericFileCommandFilterPart(filterPatterns, allFiles);

		dialogString += " 2>/dev/null ";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetYadSaveFileCommand(const std::string& title,
		                                            const std::string& defaultPathAndFile,
		                                            const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                            const bool allFiles)
	{
		return GetYadBaseFileCommand(title, defaultPathAndFile, filterPatterns, allFiles, "--file-selection --save --confirm-overwrite");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetYadOpenFileCommand(const std::string& title,
		                                            const std::string& defaultPathAndFile,
		                                            const std::vector<std::pair<std::string, std::string>>& filterPatterns,
													const bool allowMultipleSelects,
		                                            const bool allFiles)
	{
		std::string dialogAction = "--file-selection";
		if(allowMultipleSelects)
			dialogAction += " --multiple";

		return GetYadBaseFileCommand(title, defaultPathAndFile, filterPatterns, allFiles, dialogAction);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr std::string GetTKinter3FileCommandFilterPart(const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                                      const bool allFiles)
	{
		std::string dialogString{};

		if(!filterPatterns.empty() && filterPatterns[0].second[filterPatterns[0].second.size() - 1] != '*')
		{
			dialogString += "filetypes=(";
			for(const auto& [name, extensions] : filterPatterns)
			{
				if(extensions.find(';') == std::string::npos)
					dialogString += "('" + name + "',('" + extensions + "',)),";
				else
				{
					std::string exts = extensions;
					std::size_t index = 0;
					while((index = exts.find(';')) != std::string::npos)
						exts.replace(index, 1, "','");
					dialogString += "('" + name + "',('" + exts + "',)),";
				}
			}
		}

		if(allFiles && !filterPatterns.empty())
			dialogString += "('All Files','*'))";
		else if(!allFiles && !filterPatterns.empty())
			dialogString += ")";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetTKinter3SaveFileCommand(const std::string& title,
		                                                 const std::string& defaultPathAndFile,
		                                                 const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                 const bool allFiles)
	{
		std::string dialogString = Python3Name + " -S -c \"import tkinter;from tkinter import filedialog;root=tkinter.Tk();root.withdraw();";

		dialogString += "res=filedialog.asksaveasfilename(";

		if(!title.empty())
			dialogString += "title='" + title + "',";

		if(!defaultPathAndFile.empty())
		{
			std::string str = GetPathWithoutFinalSlash(defaultPathAndFile);
			if(!str.empty())
				dialogString += "initialdir='" + str + "',";
			str = GetLastName(defaultPathAndFile);
			if(!str.empty())
				dialogString += "initialfile='" + str + "',";
		}

		dialogString += GetTKinter3FileCommandFilterPart(filterPatterns, allFiles);

		dialogString += ");\nif not isinstance(res, tuple):\n\tprint(res)\n\"";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetTKinter3OpenFileCommand(const std::string& title,
		                                                 const std::string& defaultPathAndFile,
		                                                 const std::vector<std::pair<std::string, std::string>>& filterPatterns,
														 const bool allowMultipleSelects,
		                                                 const bool allFiles)
	{
		std::string dialogString = Python3Name + " -S -c \"import tkinter;from tkinter import filedialog;root=tkinter.Tk();root.withdraw();";

		dialogString += "lFiles=filedialog.askopenfilename(";

		if(allowMultipleSelects)
			dialogString += "multiple=1,";

		if(!title.empty())
			dialogString += "title='" + title + "',";

		if(!defaultPathAndFile.empty())
		{
			std::string tmp = GetPathWithoutFinalSlash(defaultPathAndFile);
			if(!tmp.empty())
				dialogString += "initialdir='" + tmp + "',";
			tmp = GetLastName(defaultPathAndFile);
			if(!tmp.empty())
				dialogString += "initialfile='" + tmp + "',";
		}

		dialogString += GetTKinter3FileCommandFilterPart(filterPatterns, allFiles);

		dialogString += ");\nif not isinstance(lFiles, tuple):\n\tprint(lFiles)\nelse:\n\tlFilesString=''\n\t";
		dialogString += "for lFile in lFiles:\n\t\tlFilesString+=str(lFile)+'|'\n\tprint(lFilesString[:-1])\n\"";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericBaseFileCommandPart(const std::string& title,
		                                                    const std::string& defaultPathAndFile,
		                                                    const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                    const bool allFiles)
	{
		std::string dialogString{};

		if(!title.empty())
			dialogString += " --title=\"" + title + "\"";
		if(!defaultPathAndFile.empty())
			dialogString += " --filename=\"" + defaultPathAndFile + "\"";
		dialogString += GetGenericFileCommandFilterPart(filterPatterns, allFiles);
		dialogString += " 2>/dev/null ";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericSaveFileCommandPart(const std::string& title,
		                                                    const std::string& defaultPathAndFile,
		                                                    const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                    const bool allFiles)
	{
		return " --file-selection --save --confirm-overwrite" + GetGenericBaseFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericOpenFileCommandPart(const std::string& title,
		                                                    const std::string& defaultPathAndFile,
		                                                    const std::vector<std::pair<std::string, std::string>>& filterPatterns,
															const bool allowMultipleSelects,
		                                                    const bool allFiles)
	{
		std::string dialogAction = " --file-selection";
		if(allowMultipleSelects)
			dialogAction += " --multiple";

		return dialogAction + GetGenericBaseFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetZenitySaveFileCommand(const std::string& title,
		                                               const std::string& defaultPathAndFile,
		                                               const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                               const bool allFiles)
	{
		std::string dialogString = "zenity ";
		if(Zenity3Present() >= 4 && XPropPresent())
			dialogString += XPropCmd;

		return dialogString + GetGenericSaveFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetZenityOpenFileCommand(const std::string& title,
		                                               const std::string& defaultPathAndFile,
		                                               const std::vector<std::pair<std::string, std::string>>& filterPatterns,
													   const bool allowMultipleSelects,
		                                               const bool allFiles)
	{
		std::string dialogString = "zenity";
		if(Zenity3Present() >= 4 && XPropPresent())
			dialogString += XPropCmd;

		return dialogString + GetGenericOpenFileCommandPart(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetMateDialogSaveFileCommand(const std::string& title,
		                                                   const std::string& defaultPathAndFile,
		                                                   const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                   const bool allFiles)
	{
		return std::string("matedialog") + GetGenericSaveFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetMateDialogOpenFileCommand(const std::string& title,
		                                                   const std::string& defaultPathAndFile,
		                                                   const std::vector<std::pair<std::string, std::string>>& filterPatterns,
														   const bool allowMultipleSelects,
		                                                   const bool allFiles)
	{
		return std::string("matedialog") + GetGenericOpenFileCommandPart(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetShellementarySaveFileCommand(const std::string& title,
		                                                      const std::string& defaultPathAndFile,
		                                                      const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                                      const bool allFiles)
	{
		return std::string("shellementary") + GetGenericSaveFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetShellementaryOpenFileCommand(const std::string& title,
		                                                      const std::string& defaultPathAndFile,
		                                                      const std::vector<std::pair<std::string, std::string>>& filterPatterns,
															  const bool allowMultipleSelects,
		                                                      const bool allFiles)
	{
		return std::string("shellementary") + GetGenericOpenFileCommandPart(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetQarmaSaveFileCommand(const std::string& title,
		                                              const std::string& defaultPathAndFile,
		                                              const std::vector<std::pair<std::string, std::string>>& filterPatterns,
		                                              const bool allFiles)
	{
		std::string dialogString = "qarma";
		if(XPropPresent())
			dialogString += " --attach$(xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)";

		return dialogString + GetGenericSaveFileCommandPart(title, defaultPathAndFile, filterPatterns, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetQarmaOpenFileCommand(const std::string& title,
		                                              const std::string& defaultPathAndFile,
		                                              const std::vector<std::pair<std::string, std::string>>& filterPatterns,
													  const bool allowMultipleSelects,
		                                              const bool allFiles)
	{
		std::string dialogString = "qarma";
		if(XPropPresent())
			dialogString += " --attach$(xprop -root 32x '\t$0' _NET_ACTIVE_WINDOW | cut -f 2)";

		return dialogString + GetGenericOpenFileCommandPart(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericSelectFolderCommandPart(const std::string& title, const std::string& defaultPath)
	{
		return " --file-selection --directory" + GetGenericBaseFileCommandPart(title, defaultPath, {}, false);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetZenitySelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		std::string dialogString = "zenity";
		if(Zenity3Present() >= 4 && XPropPresent())
			dialogString += XPropCmd;

		return dialogString + GetGenericSelectFolderCommandPart(title, defaultPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetMateDialogSelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		return "matedialog" + GetGenericSelectFolderCommandPart(title, defaultPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetShellementarySelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		return "shellementary" + GetGenericSelectFolderCommandPart(title, defaultPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetQarmaSelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		std::string dialogString = "qarma";
		if(XPropPresent())
			dialogString += XPropCmd;

		return dialogString + GetGenericSelectFolderCommandPart(title, defaultPath);
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetKDialogSelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		return GetKDialogBaseFileCommand(title, defaultPath, {}, false, "--getexistingdirectory");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetYadSelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		return GetYadBaseFileCommand(title, defaultPath, {}, false, "--file-section --directory");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetTKinter3SelectFolderCommand(const std::string& title, const std::string& defaultPath)
	{
		std::string dialogString = Python3Name;
		dialogString += " -S -c \"import tkinter;from tkinter import filedialog;root=tkinter.Tk();root.withdraw();";
		dialogString += "res=filedialog.askdirectory(";
		if(!title.empty())
			dialogString += "title='" + title + "',";
		if(!defaultPath.empty())
			dialogString += "initialdir='" + defaultPath + "'";
		dialogString += ");\nif not isinstance(res, tuple):\n\tprint(res)\n\"";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetKDialogMsgBoxCommand(const std::string& title,
													  const std::string& message,
													  const MD::Style style,
													  const MD::Buttons buttons)
	{
		std::string dialogString = "kdialog";
		if (KDialogPresent() == 2 && XPropPresent())
			dialogString += XPropCmd;

		dialogString += " --";
		if (buttons == MD::Buttons::OKCancel || buttons == MD::Buttons::YesNo)
		{
			if (style == MD::Style::Warning || style == MD::Style::Error)
				dialogString += "warning";
            dialogString += "yesno";
		}
		else if (style == MD::Style::Error)
			dialogString += "error";
		else if (style == MD::Style::Warning)
			dialogString += "sorry";
		else
			dialogString += "msgbox";
		dialogString += " \"";
		if (!message.empty())
			dialogString += message;
		dialogString += "\"";
		if (buttons == MD::Buttons::OKCancel)
			dialogString += " --yes-label OK --no-label Cancel";
        if (buttons == MD::Buttons::Quit)
            dialogString += " --ok-label Quit";
		if (!title.empty())
			dialogString += " --title \"" + title + "\"";

		dialogString += ";if [ $? = 0 ];then echo 1;else echo 0;fi";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetYadMsgBoxCommand(const std::string& title,
		                                          const std::string& message,
		                                          const MD::Style style,
		                                          const MD::Buttons buttons)
	{
		std::string dialogString = "szAnswer=$(yad --";

        if(buttons == MD::Buttons::OK)
            dialogString += "button=OK:1";
        else if(buttons == MD::Buttons::OKCancel)
            dialogString += "button=OK:1 --button=Cancel:0";
        else if(buttons == MD::Buttons::YesNo)
            dialogString += "button=Yes:1 --button=No:0";
        else if(style == MD::Style::Error)
            dialogString += "error";
        else if(style == MD::Style::Warning)
            dialogString += "warning";
        else if(style == MD::Style::Question)
            dialogString += "question";
        else
            dialogString += "info";

        if(!title.empty())
            dialogString += " --title=\"" + title + "\"";
        if(!message.empty())
            dialogString += " --text=\"" + message + "\"";

        dialogString += " 2>/dev/null ";
        dialogString += ");echo $?";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetTKinter3MsgBoxCommand(const std::string& title,
		                                               const std::string& message,
		                                               const MD::Style style,
		                                               const MD::Buttons buttons)
	{
		std::string dialogString = Python3Name;

		dialogString += " -S -c \"import tkinter;from tkinter import messagebox;root=tkinter.Tk();root.withdraw();";
		dialogString += "res=messagebox.";

		if(buttons == MD::Buttons::OKCancel)
			dialogString += "askokcancel(";
		else if(buttons == MD::Buttons::YesNo)
			dialogString += "askyesno(";
		else
			dialogString += "showinfo(";

		dialogString += "icon='";

		if(style == MD::Style::Error)
			dialogString += "error";
		else if(style == MD::Style::Question)
			dialogString += "question";
		else if(style == MD::Style::Warning)
			dialogString += "warning";
		else
			dialogString += "info";

		dialogString += "',";

		if(!title.empty())
			dialogString += "title='" + title + "',";
		if(!message.empty())
		{
			std::string msg = message;
			std::size_t p = std::string::npos;
			while((p = msg.find('\n')) != std::string::npos)
				msg.replace(p, 1, "\\n");

			dialogString += "message='" + msg + "'";
		}

		dialogString += ");\nif res is False :\n\tprint (0)\nelse :\n\tprint (1)\n\"";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericMsgBoxCommandPart(const std::string& title,
		                                                  const std::string& message,
		                                                  const MD::Style style,
		                                                  const MD::Buttons buttons,
														  const std::string& commandAction,
														  const std::string& iconCommand = "")
	{
		std::string dialogString = "szAnswer=$(" + commandAction;

		if(buttons == MD::Buttons::OKCancel)
            dialogString += "question --ok-label=OK --cancel-label=Cancel";
        else if(buttons == MD::Buttons::YesNo)
            dialogString += "question";
        else if(style == MD::Style::Error)
            dialogString += "error";
        else if(style == MD::Style::Warning)
            dialogString += "warning";
        else
            dialogString += "info";

		if(buttons == MD::Buttons::Quit)
            dialogString += " --ok-label=Quit";

		if(!title.empty())
			dialogString += " --title=\"" + title + "\"";
		if(!message.empty())
			dialogString += " --text=\"" + message + "\"";

		dialogString += iconCommand;

		dialogString += " 2>/dev/null ";
		dialogString += ");if [ $? = 0 ];then echo 1;else echo 0;fi";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetGenericMsgBoxIconCommandPart(const MD::Style style)
	{
		std::string dialogString = " --icon-name=dialog-";
		if(style == MD::Style::Question)
			dialogString += "question";
		else if(style == MD::Style::Error)
			dialogString += "error";
		else if(style == MD::Style::Warning)
			dialogString += "warning";
		else
			dialogString += "information";

		return dialogString;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetZenityMsgBoxCommand(const std::string& title,
		                                             const std::string& message,
		                                             const MD::Style style,
		                                             const MD::Buttons buttons)
	{
		std::string commandAction = "zenity";
		if(Zenity3Present() >= 4 && XPropPresent())
			commandAction += XPropCmd;

		return GetGenericMsgBoxCommandPart(title, message, style, buttons, commandAction, GetGenericMsgBoxIconCommandPart(style));
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetMateDialogMsgBoxCommand(const std::string& title,
		                                                 const std::string& message,
		                                                 const MD::Style style,
		                                                 const MD::Buttons buttons)
	{
		return GetGenericMsgBoxCommandPart(title, message, style, buttons, "matedialog");
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetShellementaryMsgBoxCommand(const std::string& title,
		                                                    const std::string& message,
		                                                    const MD::Style style,
		                                                    const MD::Buttons buttons)
	{
		return GetGenericMsgBoxCommandPart(title, message, style, buttons, "shellementary", GetGenericMsgBoxIconCommandPart(style));
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] std::string GetQarmaMsgBoxCommand(const std::string& title,
		                                            const std::string& message,
		                                            const MD::Style style,
		                                            const MD::Buttons buttons)
	{

		std::string dialogString = "qarma";
		if(XPropPresent())
			dialogString += XPropCmd;

		return GetGenericMsgBoxCommandPart(title, message, style, buttons, dialogString, GetGenericMsgBoxIconCommandPart(style));
	}

	#endif

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr bool FilenameValid(const std::string_view filenameWithoutPath)
	{
		if (filenameWithoutPath.empty())
			return false;

		return std::all_of(filenameWithoutPath.cbegin(), filenameWithoutPath.cend(), [](const char c)
		{
			return c != '\\' && c != '/' && c != ':' && c != '*' && c != '?' &&
				   c != '\"' && c != '<' && c != '>' && c != '|';
		});
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] CPP20Constexpr bool QuoteDetected(const std::string_view str)
	{
		if (str.empty())
			return false;

		if (str.find_first_of('\'') != std::string_view::npos || str.find_first_of('\"') != std::string_view::npos)
			return true;

		return false;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool DirExists(const std::string& dirPath)
	{
		std::error_code ec{};

		const bool isDir = std::filesystem::is_directory(dirPath, ec);
		if(ec != std::error_code{})
			return false;

		const bool dirExists = std::filesystem::exists(dirPath, ec);
		if(ec != std::error_code{})
			return false;

		return isDir && dirExists;
	}

	//-------------------------------------------------------------------------------------------------------------------//

	[[nodiscard]] bool FileExists(const std::string& filePathAndName)
	{
		std::error_code ec{};

		const bool isFile = std::filesystem::is_regular_file(filePathAndName, ec);
		if(ec != std::error_code{})
			return false;

		const bool fileExists = std::filesystem::exists(filePathAndName, ec);
		if(ec != std::error_code{})
			return false;

		return isFile && fileExists;
	}
}

//-------------------------------------------------------------------------------------------------------------------//

std::string MD::SaveFile(const std::string& title,
                          const std::string& defaultPathAndFile,
                          const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                          const bool allFiles)
{
	if (QuoteDetected(title))
		return SaveFile("INVALID TITLE WITH QUOTES", defaultPathAndFile, filterPatterns, allFiles);
	if (QuoteDetected(defaultPathAndFile))
		return SaveFile(title, "INVALID DEFAULT_PATH WITH QUOTES", filterPatterns, allFiles);
	for(const auto& filterPattern : filterPatterns)
	{
		if (QuoteDetected(filterPattern.first) || QuoteDetected(filterPattern.second))
			return SaveFile("INVALID FILTER_PATTERN WITH QUOTES", defaultPathAndFile, {}, allFiles);
	}

	std::string path{};
#ifdef _WIN32
	path = SaveFileWinGUI(title, defaultPathAndFile, filterPatterns, allFiles);
#else
	std::string dialogString{};
	if(KDialogPresent())
		dialogString = GetKDialogSaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(ZenityPresent())
		dialogString = GetZenitySaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(MateDialogPresent())
		dialogString = GetMateDialogSaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(ShellementaryPresent())
		dialogString = GetShellementarySaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(QarmaPresent())
		dialogString = GetQarmaSaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(YadPresent())
		dialogString = GetYadSaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);
	else if(TKinter3Present())
		dialogString = GetTKinter3SaveFileCommand(title, defaultPathAndFile, filterPatterns, allFiles);

	FILE* in = popen(dialogString.data(), "r");
	if(in == nullptr)
		return "";

	std::array<char, 128> buffer{};
	while(fgets(buffer.data(), buffer.size(), in) != nullptr)
		path += buffer.data();
	pclose(in);

	if (!path.empty() && path.back() == '\n')
		path.pop_back();
#endif

	if (path.empty())
		return "";
	std::string str = GetPathWithoutFinalSlash(path);
	if (str.empty() || !DirExists(str))
		return "";
	str = GetLastName(path);
	if (!FilenameValid(str))
		return "";

	return path;
}

//-------------------------------------------------------------------------------------------------------------------//

std::vector<std::string> MD::OpenFile(const std::string& title,
                                       const std::string& defaultPathAndFile,
                                       const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                                       const bool allowMultipleSelects,
                                       const bool allFiles)
{
	if (QuoteDetected(title))
		return OpenFile("INVALID TITLE WITH QUOTES", defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	if (QuoteDetected(defaultPathAndFile))
		return OpenFile(title, "INVALID DEFAULT_PATH WITH QUOTES", filterPatterns, allowMultipleSelects, allFiles);
	for(const auto& [fst, snd] : filterPatterns)
	{
		if (QuoteDetected(fst) || QuoteDetected(snd))
			return OpenFile("INVALID FILTER_PATTERN WITH QUOTES", defaultPathAndFile, {}, allowMultipleSelects, allFiles);
	}

	std::vector<std::string> paths{};
#ifdef _WIN32
	paths = OpenFileWinGUI(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
#else
	std::string dialogString{};
	bool wasKDialog = false;
	if(KDialogPresent())
	{
		wasKDialog = true;
		dialogString = GetKDialogOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	}
	else if(ZenityPresent())
		dialogString = GetZenityOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	else if(MateDialogPresent())
		dialogString = GetMateDialogOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	else if(ShellementaryPresent())
		dialogString = GetShellementaryOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	else if(QarmaPresent())
		dialogString = GetQarmaOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	else if(YadPresent())
		dialogString = GetYadOpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);
	else if(TKinter3Present())
		dialogString = GetTKinter3OpenFileCommand(title, defaultPathAndFile, filterPatterns, allowMultipleSelects, allFiles);

	FILE* in = popen(dialogString.data(), "r");
	if(in == nullptr)
		return {};

	std::array<char, 128> buffer{};
	std::string tmp{};
	while(fgets(buffer.data(), buffer.size(), in) != nullptr)
		tmp += buffer.data();

	pclose(in);

	if(!tmp.empty() && tmp.back() == '\n')
		tmp.pop_back();

	char separator = '|';
	if(wasKDialog)
		separator = '\n';

	if(!tmp.empty())
	{
		if(allowMultipleSelects)
		{
			tmp += separator;
			std::size_t pos = 0;
			while((pos = tmp.find(separator)) != std::string::npos)
			{
				std::string token = tmp.substr(0, pos);
				paths.push_back(std::move(token));
				tmp.erase(0, pos + 1);
			}
		}
		else
			paths.push_back(tmp);
	}
#endif

	if (paths.empty())
		return {};
	for(auto it = paths.begin(); it != paths.end(); ++it)
	{
		if(!FileExists(*it))
			it = paths.erase(it);
	}

	return paths;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string MD::OpenSingleFile(const std::string& title,
                           const std::string& defaultPathAndFile,
                           const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                           const bool allFiles)
{
	const std::vector<std::string> path = MD::OpenFile(title, defaultPathAndFile, filterPatterns, false, allFiles);
	return path.empty() ? std::string() : path[0];
}

//-------------------------------------------------------------------------------------------------------------------//

std::vector<std::string> MD::OpenMultipleFiles(const std::string& title,
                              const std::string& defaultPathAndFile,
                              const std::vector<std::pair<std::string, std::string>>& filterPatterns,
                              const bool allFiles)
{
	const std::vector<std::string> paths = MD::OpenFile(title, defaultPathAndFile, filterPatterns, true, allFiles);
	return paths.empty() ? std::vector<std::string>() : paths;
}

//-------------------------------------------------------------------------------------------------------------------//

std::string MD::SelectFolder(const std::string& title, const std::string& defaultPath)
{
	if (QuoteDetected(title))
		return MD::SelectFolder("INVALID TITLE WITH QUOTES", defaultPath);
	if (QuoteDetected(defaultPath))
		return MD::SelectFolder(title, "INVALID DEFAULT_PATH WITH QUOTES");

	std::string path{};
#ifdef _WIN32
	path = SelectFolderWinGUI(title, defaultPath);
#else
	std::string dialogString;
	if(KDialogPresent())
		dialogString = GetKDialogSelectFolderCommand(title, defaultPath);
	else if(ZenityPresent())
		dialogString = GetZenitySelectFolderCommand(title, defaultPath);
	else if(MateDialogPresent())
		dialogString = GetMateDialogSelectFolderCommand(title, defaultPath);
	else if(ShellementaryPresent())
		dialogString = GetShellementarySelectFolderCommand(title, defaultPath);
	else if(QarmaPresent())
		dialogString = GetQarmaSelectFolderCommand(title, defaultPath);
	else if(YadPresent())
		dialogString = GetYadSelectFolderCommand(title, defaultPath);
	else if(TKinter3Present())
		dialogString = GetTKinter3SelectFolderCommand(title, defaultPath);

	FILE* in = popen(dialogString.data(), "r");
	if(in == nullptr)
		return "";

	std::array<char, 128> buffer{};
	while(fgets(buffer.data(), buffer.size(), in) != nullptr)
		path += buffer.data();

	if(!path.empty() && path.back() == '\n')
		path.pop_back();

	if(!DirExists(path))
		return "";
#endif

	if (path.empty())
		return "";

	return path;
}

//-------------------------------------------------------------------------------------------------------------------//

MD::Selection MD::ShowMsgBox(const std::string& title,
							   const std::string& message,
							   const MD::Style style,
							   const MD::Buttons buttons)
{
	MD::Selection selection = MD::Selection::Error;

	if (QuoteDetected(title))
		return MD::ShowMsgBox("INVALID TITLE WITH QUOTES", message, style, buttons);
	if (QuoteDetected(message))
		return MD::ShowMsgBox(title, "INVALID DEFAULT_PATH WITH QUOTES", style, buttons);

#ifdef _WIN32
	selection = ShowMsgBoxWinGUI(title, message, style, buttons);
#else
	std::string dialogString;
	if(KDialogPresent())
		dialogString = GetKDialogMsgBoxCommand(title, message, style, buttons);
	else if(ZenityPresent())
		dialogString = GetZenityMsgBoxCommand(title, message, style, buttons);
	else if(MateDialogPresent())
		dialogString = GetMateDialogMsgBoxCommand(title, message, style, buttons);
	else if(ShellementaryPresent())
		dialogString = GetShellementaryMsgBoxCommand(title, message, style, buttons);
	else if(QarmaPresent())
		dialogString = GetQarmaMsgBoxCommand(title, message, style, buttons);
	else if(YadPresent())
		dialogString = GetYadMsgBoxCommand(title, message, style, buttons);
	else if(TKinter3Present())
		dialogString = GetTKinter3MsgBoxCommand(title, message, style, buttons);

	std::array<char, 128> buffer{};
	std::string tmp{};

	FILE* in = popen(dialogString.data(), "r");
	if(in == nullptr)
		return {};

	while(fgets(buffer.data(), buffer.size(), in) != nullptr)
		tmp += buffer.data();

	if(!tmp.empty() && tmp.back() == '\n')
		tmp.pop_back();

	if (tmp == "1")
	{
		if (buttons == MD::Buttons::YesNo)
			selection = MD::Selection::Yes;
		else if (buttons == MD::Buttons::OKCancel)
			selection = MD::Selection::OK;
		else if (buttons == MD::Buttons::OK)
			selection = MD::Selection::OK;
        else if (buttons == MD::Buttons::Quit)
            selection = MD::Selection::Quit;
	}
	else if (tmp == "0")
	{
		if (buttons == MD::Buttons::YesNo)
			selection = MD::Selection::No;
		else if (buttons == MD::Buttons::OKCancel)
			selection = MD::Selection::Cancel;
		else if (buttons == MD::Buttons::OK)
			selection = MD::Selection::Quit;
        else if (buttons == MD::Buttons::Quit)
            selection = MD::Selection::Quit;
	}
	else
		selection = MD::Selection::Quit;
#endif

	return selection;
}

MD::Selection MD::ShowMsgBox(const std::string& title, const std::string& message, const MD::Style style)
{
	return ShowMsgBox(title, message, style, MD::Buttons::OK);
}

MD::Selection MD::ShowMsgBox(const std::string& title, const std::string& message, const MD::Buttons buttons)
{
	return ShowMsgBox(title, message, MD::Style::Info, buttons);
}

MD::Selection MD::ShowMsgBox(const std::string& title, const std::string& message)
{
	return ShowMsgBox(title, message, MD::Style::Info, MD::Buttons::OK);
}
