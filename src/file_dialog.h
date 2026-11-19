#pragma once
#include <string>

// Native file dialogs and error messages. Paths are UTF-8.
// On Windows these use the system Open/Save dialogs; elsewhere they fall back
// to a fixed file name in the working directory.
namespace filedialog
{
    // Return the chosen path, or an empty string if the user cancelled.
    std::string openImage();
    std::string saveImage(const std::string& suggested);

    void showError(const std::string& title, const std::string& message);
}
