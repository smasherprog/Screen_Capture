#include "ScreenCapture.h"
#include "internal/ThreadManager.h"
#include "CGFrameProcessor.h"
#include "NSMouseProcessor.h"
#include "NSFrameProcessor.h"

namespace SL{
    namespace Screen_Capture{
        void RunCaptureMouse(std::shared_ptr<Thread_Data> data) {
            TryCaptureMouse<NSMouseProcessor>(data);
        }
        void RunCaptureMonitor(std::shared_ptr<Thread_Data> data, Monitor monitor){
            TryCaptureMonitor<NSFrameProcessor>(data, monitor);
        }
        void RunCaptureWindow(std::shared_ptr<Thread_Data> data, Window window){
            TryCaptureWindow<CGFrameProcessor>(data, window);
        }
    }
}

  
    
