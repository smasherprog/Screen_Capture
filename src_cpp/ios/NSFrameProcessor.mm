#include "NSFrameProcessorm.h"
#include <thread>
#include <chrono>
 
@implementation FrameProcessor

-(SL::Screen_Capture::DUPL_RETURN) Init:(SL::Screen_Capture::NSFrameProcessor*) parent second:(CMTime)interval
{
    self = [super init];
    if (self) {
        self.Working = false;
        self.Paused = false;
        self.nsframeprocessor = parent;
        self.avcapturesession = [[AVCaptureSession alloc] init];
       
        self.avinput = [[[AVCaptureScreenInput alloc] initWithDisplayID:SL::Screen_Capture::Id(parent->SelectedMonitor)] autorelease];
        [self.avcapturesession addInput:self.avinput];
        
        self.output = [[AVCaptureVideoDataOutput alloc] init];
        NSDictionary* videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey, nil];

        [self.output setVideoSettings:videoSettings];
        [self.output setAlwaysDiscardsLateVideoFrames:true];
        //partial capture needed
        if(parent->SelectedMonitor.OffsetX != parent->SelectedMonitor.OriginalOffsetX ||
           parent->SelectedMonitor.OffsetY != parent->SelectedMonitor.OriginalOffsetY ||
           parent->SelectedMonitor.Height != parent->SelectedMonitor.OriginalHeight ||
           parent->SelectedMonitor.Width != parent->SelectedMonitor.OriginalWidth){
                CGRect r;
                r.origin.x = parent->SelectedMonitor.OffsetX;
                //apple uses the opengl texture coords where the bottom left is 0,0
                r.origin.y = parent->SelectedMonitor.OriginalHeight - ((parent->SelectedMonitor.OffsetY - parent->SelectedMonitor.OriginalOffsetY) + parent->SelectedMonitor.Height);
                r.size.height =parent->SelectedMonitor.Height;
                r.size.width =parent->SelectedMonitor.Width;
                [self.avinput setCropRect:r];
        }
        
        [self.avinput setMinFrameDuration:interval];
        
        
        self.avinput.capturesCursor = false;
        self.avinput.capturesMouseClicks = false;
       
        [self.avcapturesession addOutput:self.output];
        [self.output setSampleBufferDelegate:self queue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0)];
        [self.avcapturesession startRunning];

        return SL::Screen_Capture::DUPL_RETURN::DUPL_RETURN_SUCCESS;
    }
    return SL::Screen_Capture::DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
}
- (void)dealloc
{
    [self.avcapturesession stopRunning];
    while(self.avcapturesession.isRunning || self.Working){
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    [self.output release];
    [self.avinput release];
    [self.avcapturesession release];
    [super dealloc];
}
-(void) setFrameRate:(CMTime)interval{
     [self.avinput setMinFrameDuration:interval];
}
-(void) Stop{
    [self.avcapturesession stopRunning];
}
-(void) Pause{
    if(self.Paused) return;
    self.Paused = true;
    if(self.output){
        self.output.connections[0].enabled = NO;
    }
}
-(void) Resume{
    if(!self.Paused) return;
    self.Paused = false;
    if(self.output){
        self.output.connections[0].enabled = YES;
    } 
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    self.Working = true;
    if(!self.avcapturesession.isRunning){
        self.Working = false;
        return;
    }
    auto data=self.nsframeprocessor->Data;
    auto& selectedmonitor =self.nsframeprocessor->SelectedMonitor;

    auto imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    auto bytesperrow = CVPixelBufferGetBytesPerRow(imageBuffer);
    auto buf = static_cast<unsigned char*>(CVPixelBufferGetBaseAddress(imageBuffer));
    SL::Screen_Capture::ProcessCapture(data->ScreenCaptureData, *(self.nsframeprocessor), selectedmonitor, buf, bytesperrow);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    self.Working = false;
}
@end

namespace SL{
    namespace Screen_Capture{
        void SetFrameInterval(FrameProcessor* f, int ms){
            
        }
        struct NSFrameProcessorImpl{
            FrameProcessor* ptr=nullptr;
            NSFrameProcessorImpl(){
                ptr = [[FrameProcessor alloc] init];
            }
            ~NSFrameProcessorImpl(){
                if(ptr) {
                    [ptr Stop];
                    auto r = CFGetRetainCount(ptr);
                    while(r!=1){
                        r = CFGetRetainCount(ptr);
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                    [ptr release];
                    ptr = nullptr;
                }
            }
            void Pause(){
                if(ptr) {
                     [ptr Pause];
                }
            }
            void Resume(){
                 if(ptr) {
                    [ptr Resume];
                }
            }
            void setMinFrameDuration(const std::chrono::microseconds& duration){
                if(duration.count()>1){
                    auto microsecondsinsec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1));
                    auto secs =std::chrono::duration_cast<std::chrono::seconds>(duration);
                    if(secs.count()>0){//more than 1 second duration. Im not going to do the math right now for that
                        [ptr setFrameRate:CMTimeMake(1, 1)];
                    } else {
                        auto f =duration.count();
                        auto f1 =microsecondsinsec.count();
                        auto interv = f1/f;
                        [ptr setFrameRate:CMTimeMake(1, interv)];
                    }
                } else {
                    [ptr setFrameRate:CMTimeMake(1, 100)];
                }
            }
            DUPL_RETURN Init(NSFrameProcessor* parent, const std::chrono::microseconds& duration){
                if(duration.count()>1){
                    auto microsecondsinsec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::seconds(1));
                    auto secs =std::chrono::duration_cast<std::chrono::seconds>(duration);
                    if(secs.count()>0){//more than 1 second duration. Im not going to do the math right now for that
                        return [ptr Init:parent second:CMTimeMake(1, 1)];
                    } else {
                        auto f =duration.count();
                        auto f1 =microsecondsinsec.count();
                        auto interv = f1/f;
                        return [ptr Init:parent second:CMTimeMake(1, interv)];
                    }
                } else {
                    return [ptr Init:parent second:CMTimeMake(1, 1000)];
                }
            }
        };
        NSFrameProcessorImpl* CreateNSFrameProcessorImpl(){
            return new NSFrameProcessorImpl();
        }
        void DestroyNSFrameProcessorImpl(NSFrameProcessorImpl* p){
            if(p){
                delete p;
            }
        }
        void setMinFrameDuration(NSFrameProcessorImpl* p, const std::chrono::microseconds& duration){
            if(p){
                p->setMinFrameDuration(duration);
            }
        }
        DUPL_RETURN Init(NSFrameProcessorImpl* createdimpl, NSFrameProcessor* parent, const std::chrono::microseconds& duration){
            if(createdimpl){
                return createdimpl->Init(parent, duration);
            }
            return DUPL_RETURN::DUPL_RETURN_ERROR_UNEXPECTED;
        }
        void Pause_(NSFrameProcessorImpl* p){
            if(p){
                return p->Pause();
            }
        }
        void Resume_(NSFrameProcessorImpl* p){
            if(p){
                return p->Resume();
            }
        }
    }
}
