#include "ScreenCapture.h"
#include "internal/ThreadManager.h"
#include "X11FrameProcessor.h"
#include "X11MouseProcessor.h"

namespace SL{
    namespace Screen_Capture{	
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data) {
            TryCaptureMouse<X11MouseProcessor>(data);
        }
        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor){
            TryCaptureMonitor<X11FrameProcessor>(data, monitor);
        }
        void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window window){
            TryCaptureWindow<X11FrameProcessor>(data, window);
        }
    }
}

  
    
