#include <iostream>

#include <ModernFileDialogs.h>

//#define SaveFileExamples 1
#define OpenFileExamples 1

int main()
{
#ifdef SaveFileExamples
	std::cout << SaveFile("TestTitle") << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test") << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"} }) << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }) << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, false) << '\n';
#endif

#ifdef OpenFileExamples
	for (const auto& str : OpenFile("TestTitle"))
		std::cout << str << '\n';
	for (const auto& str : OpenFile("TestTitle", "TestFile.Test"))
		std::cout << str << '\n';
	for (const auto& str : OpenFile("TestTitle", "TestFile.Test", {{"Test File", "*.Test;*.TS"}}))
		std::cout << str << '\n';
	for (const auto& str : OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }))
		std::cout << str << '\n';
	for (const auto& str : OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true))
		std::cout << str << '\n';
	for (const auto& str : OpenFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, true, false))
		std::cout << str << '\n';
#endif
}