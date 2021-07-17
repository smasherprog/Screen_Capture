#include "NSMouseProcessor.h"
#include "NSMouseCapture.h"
#include <iostream>

namespace SL {
namespace Screen_Capture {

    DUPL_RETURN NSMouseProcessor::Init(std::shared_ptr<Thread_Data> data)
    {
        auto ret = DUPL_RETURN::DUPL_RETURN_SUCCESS;
        Data = data;
        SLScreen_Capture_InitMouseCapture();
        return ret;
    }
    //
    // Process a given frame and its metadata
    //

    DUPL_RETURN NSMouseProcessor::ProcessFrame()
    {
        auto Ret = DUPL_RETURN_SUCCESS;

        if (Data->ScreenCaptureData.OnMouseChanged || Data->WindowCaptureData.OnMouseChanged) {
            auto mouseev = CGEventCreate(NULL);
            auto loc = CGEventGetLocation(mouseev);
            CFRelease(mouseev);

            auto imageRef = SLScreen_Capture_GetCurrentMouseImage();

            if (imageRef.Image == NULL)
                return Ret;
            auto width = CGImageGetWidth(imageRef.Image);
            auto height = CGImageGetHeight(imageRef.Image);

            auto prov = CGImageGetDataProvider(imageRef.Image);

            auto rawdatas = CGDataProviderCopyData(prov);
            auto buf = CFDataGetBytePtr(rawdatas);
            auto datalen = CFDataGetLength(rawdatas);
            if (datalen > ImageBufferSize || !OldImageBuffer) {
                ImageBuffer = std::make_unique<unsigned char[]>(datalen);
                OldImageBuffer = std::make_unique<unsigned char[]>(datalen);
            }

            memcpy(ImageBuffer.get(), buf, datalen);
            CFRelease(rawdatas);

            // this is not needed. It is freed when the image is released
            // CGDataProviderRelease(prov);

            CGImageRelease(imageRef.Image);

            ImageRect imgrect;
            imgrect.left = imgrect.top = 0;
            imgrect.right = width;
            imgrect.bottom = height;
            auto wholeimgfirst = CreateImage(imgrect, 0, reinterpret_cast<const ImageBGRA *>(ImageBuffer.get()));

            auto lastx = static_cast<int>(loc.x);
            auto lasty = static_cast<int>(loc.y);

            MousePoint mousepoint = {};
            mousepoint.Position = Point{lastx, lasty};
            mousepoint.HotSpot = Point{imageRef.HotSpotx, imageRef.HotSpoty};

            // if the mouse image is different, send the new image and swap the data

            if (memcmp(ImageBuffer.get(), OldImageBuffer.get(), datalen) != 0) {
                if (Data->ScreenCaptureData.OnMouseChanged) {
                    Data->ScreenCaptureData.OnMouseChanged(&wholeimgfirst, mousepoint);
                }
                if (Data->WindowCaptureData.OnMouseChanged) {
                    Data->WindowCaptureData.OnMouseChanged(&wholeimgfirst, mousepoint);
                }
                std::swap(ImageBuffer, OldImageBuffer);
            }
            else if (Last_x != lastx || Last_y != lasty) {
                if (Data->ScreenCaptureData.OnMouseChanged) {
                    Data->ScreenCaptureData.OnMouseChanged(nullptr, mousepoint);
                }
                if (Data->WindowCaptureData.OnMouseChanged) {
                    Data->WindowCaptureData.OnMouseChanged(nullptr, mousepoint);
                }
            }
            Last_x = lastx;
            Last_y = lasty;
        }
        return Ret;
    }

} // namespace Screen_Capture
} // namespace SL
