#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <DXGI.h>

namespace SL {
namespace Screen_Capture {

    float scaleFromDpi(int xdpi)
    {
        switch (xdpi) {
        case 96:
            return 1.0f;
            break;
        case 120:
            return 1.25f;
            break;
        case 144:
            return 1.5f;
            break;
        case 192:
            return 2.0f;
            break;
        }

        return 1.0f;
    }

    std::vector<Monitor> GetMonitors()
    {
        std::vector<Monitor> ret;

        IDXGIAdapter *pAdapter = nullptr;
        IDXGIFactory *pFactory = nullptr;

        // Create a DXGIFactory object.
        if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory))) {
            for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
                IDXGIOutput *pOutput;

                for (UINT j = 0; pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND; ++j) {
                    DXGI_OUTPUT_DESC desc;
                    pOutput->GetDesc(&desc);
                    pOutput->Release();
                    std::wstring wname = desc.DeviceName;
                    auto size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wname[0], static_cast<int>(wname.size()), NULL, 0, NULL, NULL);
                    std::string name = std::string(size, 0);
                    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wname[0], static_cast<int>(wname.size()), &name[0], static_cast<int>(size), NULL, NULL);
                    if (name.size() > static_cast<decltype(size)>(sizeof(Monitor::Name))) {
                        name.resize(sizeof(Monitor::Name));
                    }
                    DEVMODEW devMode = {};
                    EnumDisplaySettingsW(desc.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);

                    auto mon = CreateDCW(desc.DeviceName, NULL, NULL, NULL);
                    auto xdpi = GetDeviceCaps(mon, LOGPIXELSX);
                    DeleteDC(mon);
                    auto scale = scaleFromDpi(xdpi);

                    bool flipSides = desc.Rotation == DXGI_MODE_ROTATION_ROTATE90 || desc.Rotation == DXGI_MODE_ROTATION_ROTATE270;
                    ret.push_back(CreateMonitor(static_cast<int>(ret.size()), j, i, flipSides ? devMode.dmPelsWidth : devMode.dmPelsHeight,
                                                flipSides ? devMode.dmPelsHeight : devMode.dmPelsWidth, devMode.dmPosition.x, devMode.dmPosition.y,
                                                name, scale));
                }
                pAdapter->Release();
            }
            pFactory->Release();
        }
        return ret;
    }
} // namespace Screen_Capture
} // namespace SL