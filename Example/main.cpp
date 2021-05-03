/*
MIT License

Copyright (c) 2020 Jan "GamesTrap" Sch�rkamp

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

#include <ModernFileDialogs.h>

#define SaveFileExamples 1
#define OpenFileExamples 1
#define SelectFolderExamples 1
#define MessageBoxExamples 1

std::string PrintSelection(const MFD::Selection selection)
{
	switch(selection)
	{

	case MFD::Selection::Error:
		return "Error";

	case MFD::Selection::None:
		return "None";

	case MFD::Selection::Yes:
		return "Yes";

	case MFD::Selection::No:
		return "No";

	case MFD::Selection::Cancel:
		return "Cancel";

	case MFD::Selection::Quit:
		return "Quit";

	case MFD::Selection::OK:
		return "OK";

	default:
		return "Error";
	}

	return "Error";
}

int main()
{
#ifdef SaveFileExamples
	std::cout << MFD::SaveFile("TestTitle") << std::endl;
	std::cout << MFD::SaveFile("TestTitle", "TestFile.Test") << std::endl;
	std::cout << MFD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"} }) << std::endl;
	std::cout << MFD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }) << std::endl;
	std::cout << MFD::SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, false) << std::endl;
#endif

#ifdef OpenFileExamples
	for (const auto& str : MFD::OpenFile("TestTitle"))
		std::cout << str << std::endl;
	for (const auto& str : MFD::OpenFile("TestTitle", "TestFile.Test"))
		std::cout << str << std::endl;
	for (const auto& str : MFD::OpenFile("TestTitle", "TestFile.Test", {{"Test File", "*.Test;*.TS"}}))
		std::cout << str << std::endl;
	for (const auto& str : MFD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }))
		std::cout << str << std::endl;
	for (const auto& str : MFD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true))
		std::cout << str << std::endl;
	for (const auto& str : MFD::OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true, false))
		std::cout << str << std::endl;
#endif

#ifdef SelectFolderExamples
	std::cout << MFD::SelectFolder("TestTitle") << std::endl;
	std::cout << MFD::SelectFolder("TestTitle", "TestPath") << std::endl;
#endif

#ifdef MessageBoxExamples
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Info, MFD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Info, MFD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Info, MFD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Info, MFD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Error, MFD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Error, MFD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Error, MFD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Error, MFD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Question, MFD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Question, MFD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Question, MFD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Question, MFD::Buttons::YesNo)) << std::endl;

	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Warning, MFD::Buttons::OK)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Warning, MFD::Buttons::OKCancel)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Warning, MFD::Buttons::Quit)) << std::endl;
	std::cout << "Selected: " << PrintSelection(MFD::ShowMsgBox("Test Title", "Test Message\nSecond line", MFD::Style::Warning, MFD::Buttons::YesNo)) << std::endl;
#endif
}