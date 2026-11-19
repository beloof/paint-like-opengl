#include "file_dialog.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>

#include <vector>

namespace
{
std::wstring widen(const std::string& s)
{
    if (s.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

std::string narrow(const std::wstring& w)
{
    if (w.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

std::string runDialog(bool save, const std::string& suggested)
{
    std::vector<wchar_t> buffer(4096, L'\0');
    const std::wstring initial = widen(suggested);
    initial.copy(buffer.data(), buffer.size() - 1);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = buffer.data();
    ofn.nMaxFile = (DWORD)buffer.size();
    if (save)
    {
        ofn.lpstrFilter = L"PNG image (*.png)\0*.png\0";
        ofn.lpstrDefExt = L"png";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    }
    else
    {
        ofn.lpstrFilter = L"Images (*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga;*.gif\0"
                          L"All files (*.*)\0*.*\0";
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    }

    const BOOL ok = save ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
    return ok ? narrow(buffer.data()) : std::string();
}
} // namespace

namespace filedialog
{
std::string openImage() { return runDialog(false, ""); }
std::string saveImage(const std::string& suggested) { return runDialog(true, suggested); }

void showError(const std::string& title, const std::string& message)
{
    MessageBoxW(GetActiveWindow(), widen(message).c_str(), widen(title).c_str(), MB_OK | MB_ICONERROR);
}
} // namespace filedialog

#else // no native dialog: use a fixed file in the working directory

#include <cstdio>

namespace filedialog
{
std::string openImage() { return "canvas.png"; }
std::string saveImage(const std::string& suggested) { return suggested.empty() ? "canvas.png" : suggested; }

void showError(const std::string& title, const std::string& message)
{
    std::fprintf(stderr, "%s: %s\n", title.c_str(), message.c_str());
}
} // namespace filedialog

#endif
