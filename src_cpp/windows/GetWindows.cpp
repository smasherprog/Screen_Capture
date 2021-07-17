#include "GDIHelpers.h"
#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <algorithm>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>

namespace SL {
namespace Screen_Capture {

    struct srch {
        std::vector<Window> Found;
    };
    BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam)
    {
        Window w = {};

        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        w.Name[0] = '\n';
        if (pid != GetCurrentProcessId()) {
            GetWindowTextA(hwnd, w.Name, sizeof(w.Name));
        }

        srch *s = (srch *)lParam;
        w.Handle = reinterpret_cast<size_t>(hwnd);
        auto windowrect = SL::Screen_Capture::GetWindowRect(hwnd);
        w.Position.x = windowrect.ClientRect.left;
        w.Position.y = windowrect.ClientRect.top;
        w.Size.x = windowrect.ClientRect.right - windowrect.ClientRect.left;
        w.Size.y = windowrect.ClientRect.bottom - windowrect.ClientRect.top;

        std::transform(std::begin(w.Name), std::end(w.Name), std::begin(w.Name), [](char c) { return static_cast<char>(std::tolower(c)); });
        s->Found.push_back(w);
        return TRUE;
    }

    std::vector<Window> GetWindows()
    {
        srch s;
        EnumWindows(EnumWindowsProc, (LPARAM)&s);
        return s.Found;
    }

} // namespace Screen_Capture
} // namespace SL
