# ModernDialogs

[![Build CI](https://github.com/GamesTrap/ModernDialogs/actions/workflows/build.yml/badge.svg)](https://github.com/GamesTrap/ModernDialogs/actions/workflows/build.yml)
[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/GamesTrap/ModernDialogs)](https://github.com/GamesTrap/ModernDialogs)
[![GitHub repo size](https://img.shields.io/github/repo-size/GamesTrap/ModernDialogs)](https://github.com/GamesTrap/ModernDialogs)
[![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/GamesTrap/ModernDialogs?include_prereleases)](https://github.com/GamesTrap/ModernDialogs/releases)
[![GitHub All Releases](https://img.shields.io/github/downloads/GamesTrap/ModernDialogs/total)](https://github.com/GamesTrap/ModernDialogs/releases)
[![GitHub issues](https://img.shields.io/github/issues/GamesTrap/ModernDialogs)](https://github.com/GamesTrap/ModernDialogs/issues?q=is%3Aopen+is%3Aissue)
[![GitHub pull requests](https://img.shields.io/github/issues-pr/GamesTrap/ModernDialogs)](https://github.com/GamesTrap/ModernDialogs/pulls?q=is%3Aopen+is%3Apr)
[![GitHub](https://img.shields.io/github/license/GamesTrap/ModernDialogs)](https://github.com/GamesTrap/ModernDialogs/blob/master/LICENSE)

ModernDialogs (Cross-platform Linux, Windows C++17)  
OpenFileDialog, SaveFileDialog, SelectFolderDialog & MessageBox  
Supports ASCII & UTF-8

## Information

### Windows

On Windows ModernDialogs uses the WinAPI for every dialog.

### Linux

On Linux the dialogs are created by using one of the following packages if installed:

- KDialog
- Zenity
- MateDialog
- Shellementary
- Qarma
- Yad
- TKinter3

If none of these packages are installed then you will get an empty string, a vector of empty strings, or a `MD::Selection::Error` as the return value depending on the called function.

## Screenshots

Windows 10 20H2:  

OpenFile:
<br>
<img alt="OpenFileWindows" src="Images/OpenFileWindows.PNG" width="473px" height="261px">
<br>
SaveFile:
<br>
<img alt="SaveFileWindows" src="Images/SaveFileWindows.PNG" width="473px" height="261px">
<br>
SelectFolder:
<br>
<img alt="SelectFolderWindows" src="Images/SelectFolderWindows.PNG" width="320px" height="396px">
<br>
MessageBox:
<br>
<img alt="MsgBoxInfoWindows" src="Images/MsgBoxInfoWindows.PNG" width="163px" height="152px">
<img alt="MsgBoxQuestionWindows" src="Images/MsgBoxQuestionWindows.PNG" width="163px" height="152px">
<img alt="MsgBoxWarningWindows" src="Images/MsgBoxWarningWindows.PNG" width="211px" height="152px">
<img alt="MsgBoxErrorWindows" src="Images/MsgBoxErrorWindows.PNG" width="211px" height="152px">

Ubuntu 20.10 X11 Gnome 3:  

OpenFile:
<br>
<img alt="OpenFileLinux" src="Images/OpenFileLinux.PNG" width="508px" height="352px">
<br>
SaveFile:
<br>
<img alt="SaveFileLinux" src="Images/SaveFileLinux.PNG" width="508px" height="352px">
<br>
SelectFolder:
<br>
<img alt="SelectFolderLinux" src="Images/SelectFolderLinux.PNG" width="508px" height="352px">
<br>
MessageBox:
<br>
<img alt="MsgBoxInfoLinux" src="Images/MsgBoxInfoLinux.PNG" width="142px" height="137px">
<img alt="MsgBoxQuestionLinux" src="Images/MsgBoxQuestionLinux.PNG" width="130px" height="137px">
<img alt="MsgBoxWarningLinux" src="Images/MsgBoxWarningLinux.PNG" width="160px" height="137px">
<img alt="MsgBoxErrorLinux" src="Images/MsgBoxErrorLinux.PNG" width="160px" height="137px">
## Setup

First clone the repository with `git clone https://github.com/GamesTrap/ModernDialogs`.

Then, execute one of the generator scripts in the GeneratorScripts folder.

## License

MIT License

Copyright (c) 2020-2022 Jan "GamesTrap" Schürkamp

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
