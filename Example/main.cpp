/*
MIT License

Copyright (c) 2020 - 2022 Jan "GamesTrap" Sch�rkamp

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

#include <iostream>

#include <ModernDialogs.h>

#define SaveFileExamples 1
#define OpenFileExamples 1
#define SelectFolderExamples 1
#define MessageBoxExamples 1

std::string PrintSelection(const MD::Selection selection)
{
	switch(selection)
	{

	case MD::Selection::Error:
		return "Error";

	case MD::Selection::None:
		return "None";

	case MD::Selection::Yes:
		return "Yes";

	case MD::Selection::No:
		return "No";

	case MD::Selection::Cancel:
		return "Cancel";

	case MD::Selection::Quit:
		return "Quit";

	case MD::Selection::OK:
		return "OK";

	default:
		return "Error";
	}
}

int main()
{
#ifdef SaveFileExamples
	std::cout << MD::SaveFile("TestTitle") << std::endl;
	std::cout << MD::SaveFile("TestTitle", "TestFile.Test") << std::endl;
	std::cout << MD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"} }) << std::endl;
	std::cout << MD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }) << std::endl;
	std::cout << MD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, false) << std::endl;
#endif

#ifdef OpenFileExamples
	for (const auto& str : MD::OpenFile("TestTitle"))
		std::cout << str << std::endl;
	for (const auto& str : MD::OpenFile("TestTitle", "TestFile.Test"))
		std::cout << str << std::endl;
	for (const auto& str : MD::OpenFile("TestTitle", "TestFile.Test", {{"Test File", "*.Test;*.TS"}}))
		std::cout << str << std::endl;
	for (const auto& str : MD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }))
		std::cout << str << std::endl;
	for (const auto& str : MD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true))
		std::cout << str << std::endl;
	for (const auto& str : MD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true, false))
		std::cout << str << std::endl;
#endif

#ifdef SelectFolderExamples
	std::cout << MD::SelectFolder("TestTitle") << std::endl;
	std::cout << MD::SelectFolder("TestTitle", "TestPath") << std::endl;
#endif

#ifdef MessageBoxExamples
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Info, MD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Info, MD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Info, MD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Info, MD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Error, MD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Error, MD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Error, MD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Error, MD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Question, MD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Question, MD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Question, MD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Question, MD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Warning, MD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Warning, MD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Warning, MD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MD::ShowMsgBox("Test Title", "Test Message\nSecond line", MD::Style::Warning, MD::Buttons::YesNo)) << std::endl;
#endif
}