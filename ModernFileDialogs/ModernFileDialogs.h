/*
MIT License

Copyright (c) 2020 Jan "GamesTrap" Schürkamp

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

#ifndef _GAMESTRAP_MODERNFILEDIALOGS_H_
#define _GAMESTRAP_MODERNFILEDIALOGS_H_

#include <string>
#include <vector>

//TODO Document
//TODO Multiple filters under the same name should be splitted by ';'s
//TODO Example: {"SomeFile", "*.SF;*.SomeFile}
std::string SaveFile(const std::string& title,
                     const std::string& defaultPathAndFile = "",
                     const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                     bool allFiles = true);

//-------------------------------------------------------------------------------------------------------------------//
//
//TODO Document
//TODO Multiple filters under the same name should be splitted by ';'s
//TODO Example: {"SomeFile", "*.SF;*.SomeFile}
std::vector<std::string> OpenFile(const std::string& title,
                                  const std::string& defaultPathAndFile = "",
                                  const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                                  bool allowMultipleSelects = false,
                                  bool allFiles = true);

//TODO Document
//TODO Single file alias for OpenFile
std::string OpenSingleFile(const std::string& title,
                           const std::string& defaultPathAndFile = "",
                           const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                           bool allFiles = true);

//TODO Document
//TODO Multi file alias for OpenFile
std::vector<std::string> OpenMultipleFiles(const std::string& title,
							               const std::string& defaultPathAndFile = "",
							               const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
							               bool allFiles = true);

#endif /*_GAMESTRAP_MODERNFILEDIALOGS_H_*/