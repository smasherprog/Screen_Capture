#pragma once

#if defined(WINDOWS) || defined(WIN32)
#if defined(SC_LITE_DLL)
#define SC_LITE_C_EXTERN extern "C" __declspec(dllexport)
#define SC_LITE_EXTERN __declspec(dllexport)
#else
#define SC_LITE_C_EXTERN
#define SC_LITE_EXTERN
#endif
#else
#if defined(SC_LITE_DLL)
#define SC_LITE_C_EXTERN extern "C"
#define SC_LITE_EXTERN
#else
#define SC_LITE_C_EXTERN
#define SC_LITE_EXTERN
#endif
#endif

#include <assert.h>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace SL {
namespace Screen_Capture {
    namespace C_API {
        class IScreenCaptureManagerWrapper;
        class ICaptureConfigurationScreenCaptureCallbackWrapper;
    }; // namespace C_API
    struct SC_LITE_EXTERN Point {
        int x;
        int y;
    };
    struct SC_LITE_EXTERN MousePoint {
        Point Position;
        Point HotSpot;
    };
    struct SC_LITE_EXTERN Window {
        size_t Handle;
        Point Position;

        Point Size;
        // Name will always be lower case. It is converted to lower case internally by the library for comparisons
        char Name[128] = {0};
    };
    struct SC_LITE_EXTERN Monitor {
        int Id = INT32_MAX;
        int Index = INT32_MAX;
        int Adapter = INT32_MAX;
        int Height = 0;
        int Width = 0;
        int OriginalHeight = 0;
        int OriginalWidth = 0;
        // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
        // monitors around so this affects their offset.
        int OffsetX = 0;
        int OffsetY = 0;
        int OriginalOffsetX = 0;
        int OriginalOffsetY = 0;
        char Name[128] = {0};
        float Scaling = 1.0f;
    };

    struct Image;
    struct ImageBGRA {
        unsigned char B, G, R, A;
    };

    // index to self in the GetMonitors() function
    SC_LITE_EXTERN int Index(const Monitor &mointor);
    // unique identifier
    SC_LITE_EXTERN int Id(const Monitor &mointor);
    SC_LITE_EXTERN int Adapter(const Monitor &mointor);
    SC_LITE_EXTERN int OffsetX(const Monitor &mointor);
    SC_LITE_EXTERN int OffsetY(const Monitor &mointor);
    SC_LITE_EXTERN void OffsetX(Monitor &mointor, int x);
    SC_LITE_EXTERN void OffsetY(Monitor &mointor, int y);
    SC_LITE_EXTERN int OffsetX(const Window &mointor);
    SC_LITE_EXTERN int OffsetY(const Window &mointor);
    SC_LITE_EXTERN void OffsetX(Window &mointor, int x);
    SC_LITE_EXTERN void OffsetY(Window &mointor, int y);
    SC_LITE_EXTERN int OffsetX(const Image &img);
    SC_LITE_EXTERN int OffsetY(const Image &img);
    SC_LITE_EXTERN const char *Name(const Monitor &mointor);
    SC_LITE_EXTERN const char *Name(const Window &mointor);
    SC_LITE_EXTERN int Height(const Monitor &mointor);
    SC_LITE_EXTERN int Width(const Monitor &mointor);
    SC_LITE_EXTERN void Height(Monitor &mointor, int h);
    SC_LITE_EXTERN void Width(Monitor &mointor, int w);
    SC_LITE_EXTERN int Height(const Window &mointor);
    SC_LITE_EXTERN int Width(const Window &mointor);
    SC_LITE_EXTERN void Height(Window &mointor, int h);
    SC_LITE_EXTERN void Width(Window &mointor, int w);
    SC_LITE_EXTERN int Height(const Image &img);
    SC_LITE_EXTERN int Width(const Image &img);
    SC_LITE_EXTERN int X(const Point &p);
    SC_LITE_EXTERN int Y(const Point &p);

    // the start of the image data, this is not guarenteed to be contiguous.
    SC_LITE_EXTERN const ImageBGRA *StartSrc(const Image &img);
    SC_LITE_EXTERN const ImageBGRA *GotoNextRow(const Image &img, const ImageBGRA *current);
    SC_LITE_EXTERN bool isDataContiguous(const Image &img);
    /*
        this is the ONLY funcion for pulling data out of the Image object and is layed out here in the header so that
        users can see how to extra data and convert it to their own needed format. Initially, I included custom extract functions
        but this is beyond the scope of this library. You must copy the image data if you want to use it as the library owns the Image Data.
    */
    inline void Extract(const Image &img, unsigned char *dst, size_t dst_size)
    {
        assert(dst_size >= static_cast<size_t>(Width(img) * Height(img) * sizeof(ImageBGRA)));
        auto startdst = dst;
        auto startsrc = StartSrc(img);
        if (isDataContiguous(img)) { // no padding, the entire copy can be a single memcpy call
            memcpy(startdst, startsrc, Width(img) * Height(img) * sizeof(ImageBGRA));
        }
        else {
            for (auto i = 0; i < Height(img); i++) {
                memcpy(startdst, startsrc, sizeof(ImageBGRA) * Width(img));
                startdst += sizeof(ImageBGRA) * Width(img); // advance to the next row
                startsrc = GotoNextRow(img, startsrc);      // advance to the next row
            }
        }
    }

    class Timer {
        using Clock =
            std::conditional<std::chrono::high_resolution_clock::is_steady, std::chrono::high_resolution_clock, std::chrono::steady_clock>::type;

        std::chrono::microseconds Duration;
        Clock::time_point Deadline;

      public:
        template <typename Rep, typename Period>
        Timer(const std::chrono::duration<Rep, Period> &duration)
            : Duration(std::chrono::duration_cast<std::chrono::microseconds>(duration)), Deadline(Clock::now() + Duration)
        {
        }
        void start() { Deadline = Clock::now() + Duration; }
        void wait()
        {
            const auto now = Clock::now();
            if (now < Deadline) {
                std::this_thread::sleep_for(Deadline - now);
            }
        }
        std::chrono::microseconds duration() const { return Duration; }
    };
    // will return all attached monitors

    SC_LITE_EXTERN std::vector<Monitor> GetMonitors();
    // will return all windows
    SC_LITE_EXTERN std::vector<Window> GetWindows();
    namespace C_API {
        //GetWindows and GetMonitors expect a pre allocated buffer with the size as the second input parameter. 
        //The output of these functions is the actual total number of elements that the library had to return. So, applications should use this value in determininng how to preallocate data.
        SC_LITE_C_EXTERN int GetWindows(Window *windows, int windows_size);
        SC_LITE_C_EXTERN int GetMonitors(Monitor *monitors, int monitors_size);
        SC_LITE_C_EXTERN bool isMonitorInsideBounds(const Monitor *monitors, const int monitorsize, const Monitor *monitor);
    }; // namespace C_API
    SC_LITE_EXTERN bool isMonitorInsideBounds(const std::vector<Monitor> &monitors, const Monitor &monitor);
    typedef std::function<void(const SL::Screen_Capture::Image &img, const Window &window)> WindowCaptureCallback;
    typedef std::function<void(const SL::Screen_Capture::Image &img, const Monitor &monitor)> ScreenCaptureCallback;
    typedef std::function<void(const SL::Screen_Capture::Image *img, const MousePoint &mousepoint)> MouseCallback;
    typedef std::function<std::vector<Monitor>()> MonitorCallback;
    typedef std::function<std::vector<Window>()> WindowCallback;

    class SC_LITE_EXTERN IScreenCaptureManager {
      public:
        virtual ~IScreenCaptureManager() {}

        // Used by the library to determine the callback frequency
        template <class Rep, class Period> void setFrameChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
        {
            setFrameChangeInterval(std::make_shared<Timer>(rel_time));
        }
        // Used by the library to determine the callback frequency
        template <class Rep, class Period> void setMouseChangeInterval(const std::chrono::duration<Rep, Period> &rel_time)
        {
            setMouseChangeInterval(std::make_shared<Timer>(rel_time));
        }

        virtual void setFrameChangeInterval(const std::shared_ptr<Timer> &timer) = 0;
        virtual void setMouseChangeInterval(const std::shared_ptr<Timer> &timer) = 0;

        // Will pause all capturing
        virtual void pause() = 0;
        // Will return whether the library is paused
        virtual bool isPaused() const = 0;
        // Will resume all capturing if paused, otherwise has no effect
        virtual void resume() = 0;
    };

    template <typename CAPTURECALLBACK> class ICaptureConfiguration {
      public:
        virtual ~ICaptureConfiguration() {}
        // When a new frame is available the callback is invoked
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onNewFrame(const CAPTURECALLBACK &cb) = 0;
        // When a change in a frame is detected, the callback is invoked
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onFrameChanged(const CAPTURECALLBACK &cb) = 0;
        // When a mouse image changes or the mouse changes position, the callback is invoked.
        virtual std::shared_ptr<ICaptureConfiguration<CAPTURECALLBACK>> onMouseChanged(const MouseCallback &cb) = 0;
        // start capturing
        virtual std::shared_ptr<IScreenCaptureManager> start_capturing() = 0;
    };

    namespace C_API {
        typedef int (*C_API_ScreenCaptureCallback)(const Image &img, const Monitor &monitor); 
        SC_LITE_C_EXTERN void onNewFrame(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr, C_API_ScreenCaptureCallback cb);
        SC_LITE_C_EXTERN void onFrameChanged(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr, C_API_ScreenCaptureCallback cb);
        SC_LITE_C_EXTERN IScreenCaptureManagerWrapper *start_capturing(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr);
        SC_LITE_C_EXTERN void FreeIScreenCaptureManagerWrapper(IScreenCaptureManagerWrapper *ptr);

        SC_LITE_C_EXTERN void setFrameChangeInterval(IScreenCaptureManagerWrapper *ptr, int milliseconds);
        SC_LITE_C_EXTERN void pause(IScreenCaptureManagerWrapper *ptr);
        SC_LITE_C_EXTERN bool isPaused(IScreenCaptureManagerWrapper *ptr);
        SC_LITE_C_EXTERN void resume(IScreenCaptureManagerWrapper *ptr);
    }; // namespace C_API

    // the callback of windowstocapture represents the list of monitors which should be captured. Users should return the list of monitors they want
    // to be captured
    SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<ScreenCaptureCallback>> CreateCaptureConfiguration(const MonitorCallback &monitorstocapture);
    namespace C_API { 
        typedef int (*C_API_MonitorCallback)(Monitor *monitorbuffer, int monitorbuffersize); 
        SC_LITE_C_EXTERN ICaptureConfigurationScreenCaptureCallbackWrapper *CreateCaptureConfiguration(C_API_MonitorCallback monitorstocapture);
        SC_LITE_C_EXTERN void FreeCaptureConfiguration(ICaptureConfigurationScreenCaptureCallbackWrapper *ptr);
    }; // namespace C_API
    // the callback of windowstocapture represents the list of windows which should be captured. Users should return the list of windows they want to
    // be captured
    SC_LITE_EXTERN std::shared_ptr<ICaptureConfiguration<WindowCaptureCallback>> CreateCaptureConfiguration(const WindowCallback &windowstocapture);
} // namespace Screen_Capture
} // namespace SL
