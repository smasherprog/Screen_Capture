#include "internal/ThreadManager.h"
#include <assert.h>
#include <algorithm>
#include <fstream>

SL::Screen_Capture::ThreadManager::ThreadManager()
{
}
SL::Screen_Capture::ThreadManager::~ThreadManager()
{
    Join();
}
inline std::ostream &operator<<(std::ostream &os, const SL::Screen_Capture::Monitor &p)
{
    return os << "Id=" << p.Id << " Index=" << p.Index << " Height=" << p.Height << " Width=" << p.Width << " OffsetX=" << p.OffsetX
              << " OffsetY=" << p.OffsetY << " Name=" << p.Name;
}
void SL::Screen_Capture::ThreadManager::Init(const std::shared_ptr<Thread_Data>& data)
{
    assert(m_ThreadHandles.empty());

    if (data->ScreenCaptureData.getThingsToWatch) {
        auto monitors = data->ScreenCaptureData.getThingsToWatch();
        auto mons = GetMonitors();
        for (auto &m : monitors) {
            assert(isMonitorInsideBounds(mons, m));
        }

        m_ThreadHandles.resize(monitors.size() + (data->ScreenCaptureData.OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed

        for (size_t i = 0; i < monitors.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureMonitor, data, monitors[i]);
        }
        if (data->ScreenCaptureData.OnMouseChanged) {
            m_ThreadHandles.back() = std::thread([data] {
                SL::Screen_Capture::RunCaptureMouse(data);
            });
        }

    }
    else if (data->WindowCaptureData.getThingsToWatch) {
        auto windows = data->WindowCaptureData.getThingsToWatch();
        m_ThreadHandles.resize(windows.size() + (data->WindowCaptureData.OnMouseChanged ? 1 : 0)); // add another thread for mouse capturing if needed
        for (size_t i = 0; i < windows.size(); ++i) {
            m_ThreadHandles[i] = std::thread(&SL::Screen_Capture::RunCaptureWindow, data, windows[i]);
        }
        if (data->WindowCaptureData.OnMouseChanged) {
            m_ThreadHandles.back() = std::thread([data] {
                SL::Screen_Capture::RunCaptureMouse(data);
            });
        }
    }
}

void SL::Screen_Capture::ThreadManager::Join()
{
    for (auto& t : m_ThreadHandles) {
        if (t.joinable()) {
            if (t.get_id() == std::this_thread::get_id()) {
                t.detach();// will run to completion
            }
            else {
                t.join();
            }
        }
    }
    m_ThreadHandles.clear();
}
