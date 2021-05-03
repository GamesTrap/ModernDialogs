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

#ifndef _GAMESTRAP_MODERNDIALOGS_H_
#define _GAMESTRAP_MODERNDIALOGS_H_

#include <string>
#include <vector>

namespace MD
{
    /// <summary>
    /// Open a Save File Dialog.
    /// </summary>
    /// <param name="title">Title for the Dialog.</param>
    /// <param name="defaultPathAndFile">Sets a default path and file.</param>
    /// <param name="filterPatterns">File filters (Separate multiple extensions for the same filter with a ';'. Example: {"Test File", "*.Test;*.TS"}.</param>
    /// <param name="allFiles">Whether to add a filter for "All Files (*.*)" or not.</param>
    /// <returns>Path of the Dialog or empty string.</returns>
    std::string SaveFile(const std::string& title,
                         const std::string& defaultPathAndFile = "",
                         const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                         bool allFiles = true);

    //-------------------------------------------------------------------------------------------------------------------//

    /// <summary>
    /// Opens an Open File Dialog.
    /// </summary>
    /// <param name="title">Title for the Dialog.</param>
    /// <param name="defaultPathAndFile">Sets a default path and file.</param>
    /// <param name="filterPatterns">File filters (Separate multiple extensions for the same filter with a ';'. Example: {"Test File", "*.Test;*.TS"}.</param>
    /// <param name="allowMultipleSelects">Whether to allow multiple file selections or not.</param>
    /// <param name="allFiles">Whether to add a filter for "All Files (*.*)" or not.</param>
    /// <returns>Path(s) of the Dialog or empty string or vector.</returns>
    std::vector<std::string> OpenFile(const std::string& title,
                                      const std::string& defaultPathAndFile = "",
                                      const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                                      bool allowMultipleSelects = false,
                                      bool allFiles = true);

    /// <summary>
    /// Opens an Open File Dialog for a single file.<br>
    /// Alias for OpenFile();
    /// </summary>
    /// <param name="title">Title for the Dialog.</param>
    /// <param name="defaultPathAndFile">Sets a default path and file.</param>
    /// <param name="filterPatterns">File filters (Separate multiple extensions for the same filter with a ';'. Example: {"Test File", "*.Test;*.TS"}.</param>
    /// <param name="allFiles">Whether to add a filter for "All Files (*.*)" or not.</param>
    /// <returns>Path of the Dialog or empty string.</returns>
    std::string OpenSingleFile(const std::string& title,
                               const std::string& defaultPathAndFile = "",
                               const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                               bool allFiles = true);

    /// <summary>
    /// Opens an Open File Dialog for multiple files.<br>
    /// Alias for OpenFile();
    /// </summary>
    /// <param name="title">Title for the Dialog.</param>
    /// <param name="defaultPathAndFile">Sets a default path and file.</param>
    /// <param name="filterPatterns">File filters (Separate multiple extensions for the same filter with a ';'. Example: {"Test File", "*.Test;*.TS"}.</param>
    /// <param name="allFiles">Whether to add a filter for "All Files (*.*)" or not.</param>
    /// <returns>Paths of the Dialog or empty vector.</returns>
    std::vector<std::string> OpenMultipleFiles(const std::string& title,
                                               const std::string& defaultPathAndFile = "",
                                               const std::vector<std::pair<std::string, std::string>>& filterPatterns = {},
                                               bool allFiles = true);

    //-------------------------------------------------------------------------------------------------------------------//

    /// <summary>
    /// Opens an Select Folder Dialog.
    /// </summary>
    /// <param name="title">Title for the Dialog.</param>
    /// <param name="defaultPath">Sets a default path and file.</param>
    /// <returns>Path of the Select Folder Dialog or empty string.</returns>
    std::string SelectFolder(const std::string& title, const std::string& defaultPath = "");

    //-------------------------------------------------------------------------------------------------------------------//

    /// <summary>
    /// Styles for message boxes
    /// </summary>
    enum class Style
    {
        Info,
        Warning,
        Error,
        Question
    };

    /// <summary>
    /// Buttons for message boxes
    /// </summary>
    enum class Buttons
    {
        OK,
        OKCancel,
        YesNo,
        Quit
    };

    /// <summary>
    /// Possible responses from a message box.<br>
    /// "None" signifies that no option was selected.<br>
    /// "Error" signifies that an error was encountered while creating the message box.
    /// </summary>
    enum class Selection
    {
        Error = 0,
        None,

        OK,
        Cancel,
        Yes,
        No,
        Quit
    };

    /// <summary>
    /// Blocking call to create a modal message box with the given title, message, style and buttons.
    /// </summary>
    /// 
    /// <param name="title">Title for the message box.</param>
    /// <param name="message">Message for the message box.</param>
    /// <param name="style">Style for the message box.</param>
    /// <param name="buttons">Button(s) for the message box.</param>
    /// <returns>Selection made by the user.</returns>
    Selection ShowMsgBox(const std::string& title, const std::string& message, Style style, Buttons buttons);

    /// <summary>
    /// Convenience function to call ShowMsgBox() with default button.
    /// </summary>
    /// 
    /// <param name="title">Title for the message box.</param>
    /// <param name="message">Message for the message box.</param>
    /// <param name="style">Style for the message box.</param>
    /// <returns>Selection made by the user.</returns>
    Selection ShowMsgBox(const std::string& title, const std::string& message, Style style);

    /// <summary>
    /// Convenience function to call ShowMsgBox() with default style.
    /// </summary>
    /// 
    /// <param name="title">Title for the message box.</param>
    /// <param name="message">Message for the message box.</param>
    /// <param name="buttons">Button(s) for the message box.</param>
    /// <returns>Selection made by the user.</returns>
    Selection ShowMsgBox(const std::string& title, const std::string& message, Buttons buttons);

    /// <summary>
    /// Convenience function to call ShowMsgBox() with default style and button.
    /// </summary>
    /// 
    /// <param name="title">Title for the message box.</param>
    /// <param name="message">Message for the message box.</param>
    /// <returns>Selection made by the user.</returns>
    Selection ShowMsgBox(const std::string& title, const std::string& message);
}

#endif /*_GAMESTRAP_MODERNDIALOGS_H_*/