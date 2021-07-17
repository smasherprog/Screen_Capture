#include "NSFrameProcessor.h"

namespace SL {
    namespace Screen_Capture {
   
        NSFrameProcessor::NSFrameProcessor(){
            NSFrameProcessorImpl_ = nullptr;
        }
        NSFrameProcessor::~NSFrameProcessor(){
            DestroyNSFrameProcessorImpl(NSFrameProcessorImpl_);
        }
        DUPL_RETURN NSFrameProcessor::Init(std::shared_ptr<Thread_Data> data, Monitor &monitor)
        {
            Data = data;
            auto timer = std::atomic_load(&Data->ScreenCaptureData.FrameTimer);
            LastDuration = timer->duration();
            SelectedMonitor = monitor;
            NSFrameProcessorImpl_ = CreateNSFrameProcessorImpl();
            return  Screen_Capture::Init(NSFrameProcessorImpl_, this, LastDuration);
        }
         
        DUPL_RETURN NSFrameProcessor::ProcessFrame(const Monitor &curentmonitorinfo)
        {
            auto timer = std::atomic_load(&Data->ScreenCaptureData.FrameTimer);
            //get the timer and check if we need to update the internal timer
            if(timer->duration()!= LastDuration){
                LastDuration = timer->duration();
                setMinFrameDuration(NSFrameProcessorImpl_, LastDuration);
            }
            return DUPL_RETURN_SUCCESS;
        }
        void NSFrameProcessor::Pause(){
            Pause_(NSFrameProcessorImpl_);
        }
        void NSFrameProcessor::Resume(){
              Resume_(NSFrameProcessorImpl_);
        }
    }
}
