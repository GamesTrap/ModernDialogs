#include <iostream>

#include <ModernFileDialogs.h>

#define SaveFileExamples 1

int main()
{
#ifdef SaveFileExamples
	std::cout << SaveFile("TestTitle") << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test") << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"", "*.Test;*.TS"} }) << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }) << '\n';
	std::cout << SaveFile("TestTitle", "TestFile.Test", { {"Test File", "*.Test;*.TS"}, {"Another Test File", "*.ATS"} }, false) << '\n';
#endif
}