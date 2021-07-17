#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <algorithm>
#include <string>
#include "TargetConditionals.h"
#include <ApplicationServices/ApplicationServices.h>
#include <iostream>

namespace SL
{
namespace Screen_Capture
{

    std::vector<Window> GetWindows()
    {
        CGDisplayCount count=0;
        CGGetActiveDisplayList(0, 0, &count);
        std::vector<CGDirectDisplayID> displays;
        displays.resize(count);
        CGGetActiveDisplayList(count, displays.data(), &count);
        auto xscale=1.0f;
        auto yscale = 1.0f;
        
        for(auto  i = 0; i < count; i++) {
            //only include non-mirrored displays
            if(CGDisplayMirrorsDisplay(displays[i]) == kCGNullDirectDisplay){
                
                auto dismode =CGDisplayCopyDisplayMode(displays[i]);
                auto scaledsize = CGDisplayBounds(displays[i]);
            
                auto pixelwidth = CGDisplayModeGetPixelWidth(dismode);
                auto pixelheight = CGDisplayModeGetPixelHeight(dismode);
                
                CGDisplayModeRelease(dismode);
                
                if(scaledsize.size.width !=pixelwidth){//scaling going on!
                    xscale = static_cast<float>(pixelwidth)/static_cast<float>(scaledsize.size.width);
                }
                if(scaledsize.size.height !=pixelheight){//scaling going on!
                    yscale = static_cast<float>(pixelheight)/static_cast<float>(scaledsize.size.height);
                }
                break;
            }
        }
        
        auto windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
        std::vector<Window> ret;
        CFIndex numWindows = CFArrayGetCount(windowList );
   
        for( int i = 0; i < (int)numWindows; i++ ) {
            Window w = {};
            uint32_t windowid=0;
            auto dict = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
            auto cfwindowname = static_cast<CFStringRef>(CFDictionaryGetValue(dict, kCGWindowName));
            CFStringGetCString(cfwindowname, w.Name, sizeof(w.Name), kCFStringEncodingUTF8);
            w.Name[sizeof(w.Name)-1] = '\n';
     
         
            CFNumberGetValue(static_cast<CFNumberRef>(CFDictionaryGetValue(dict, kCGWindowNumber)), kCFNumberIntType, &windowid);
            w.Handle = static_cast<size_t>(windowid);
               
            auto dims =static_cast<CFDictionaryRef>(CFDictionaryGetValue(dict,kCGWindowBounds));
            CGRect rect;
            CGRectMakeWithDictionaryRepresentation(dims, &rect);
            w.Position.x = static_cast<int>(rect.origin.x);
            w.Position.y = static_cast<int>(rect.origin.y);
                
            w.Size.x = static_cast<int>(rect.size.width * xscale);
            w.Size.y = static_cast<int>(rect.size.height* yscale);  
	        std::transform(std::begin(w.Name), std::end(w.Name), std::begin(w.Name), ::tolower); 
            ret.push_back(w);
        }
        CFRelease(windowList);
        return ret;
    }
}
}
