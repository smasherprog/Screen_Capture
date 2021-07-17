//
//  NSMouseCapture.m
//  Screen_Capture
//
//  Created by scott lee on 1/11/17.
//
//

#import <Foundation/Foundation.h>
#import <appkit/appkit.h>
#import "NSMouseCapture.h"

void SLScreen_Capture_InitMouseCapture(){
    [NSApplication sharedApplication];
}
struct SL_MouseCur SLScreen_Capture_GetCurrentMouseImage(){
	struct SL_MouseCur ret= {}; 
 
    @autoreleasepool {
        NSCursor *cur = [NSCursor currentSystemCursor];
        if(cur==nil) return ret;
        NSImage *overlay    =  [cur image];
        CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[overlay TIFFRepresentation], NULL);
        ret.Image = CGImageSourceCreateImageAtIndex(source, 0, NULL);
		NSPoint p = [cur hotSpot];
		ret.HotSpotx = p.x;
		ret.HotSpoty = p.y;
        CFRelease(source);
    }
 
    return ret;
} 