#include "ScreenCapture.h"
#include "internal/SCCommon.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>


namespace {
    class UniqueTextProperty {
    public:
        UniqueTextProperty()
        {
            p.value = nullptr;
        }

        UniqueTextProperty(const UniqueTextProperty&) = delete;
        UniqueTextProperty& operator=(const UniqueTextProperty&) = delete;

        UniqueTextProperty(UniqueTextProperty&& other):
            p{other.p}
        {
            other.p = XTextProperty{};
        }

        UniqueTextProperty& operator=(UniqueTextProperty&& other)
        {
            swap(*this, other);
            return *this;
        }

        friend void swap(UniqueTextProperty& lhs, UniqueTextProperty& rhs) {
            using std::swap;
            swap(lhs.p, rhs.p);
        }

        ~UniqueTextProperty()
        {
            if (p.value) {
                XFree(p.value);
            }
        }

        auto& get() {
            return p;
        }

    private:
        XTextProperty p;
    };

    auto GetWMName(Display* display, Window window)
    {
        auto x = UniqueTextProperty{};
        XGetWMName(display, window, &x.get());
        return x;
    }

    auto TextPropertyToStrings(
        Display* dpy,
        const XTextProperty& prop
    )
    {
        char **list;
        auto n_strings = 0;
        auto result = std::vector<std::string>{};

        auto status = XmbTextPropertyToTextList(
            dpy,
            &prop,
            &list,
            &n_strings
        );

        if (status < Success or not n_strings or not *list) {
            return result;
        }

        for (auto i = 0; i < n_strings; ++i) {
            result.emplace_back(list[i]);
        }

        XFreeStringList(list);

        return result;
    }
}

namespace SL
{
namespace Screen_Capture
{

    void AddWindow(Display* display, XID& window, std::vector<Window>& wnd)
    {
        using namespace std::string_literals;

        auto wm_name = GetWMName(display, window);
        auto candidates = TextPropertyToStrings(display, wm_name.get());
        Window w = {};
        w.Handle = static_cast<size_t>( (size_t) window);

        XWindowAttributes wndattr;
        XGetWindowAttributes(display, window, &wndattr);

        w.Position = Point{ wndattr.x, wndattr.y };
        w.Size = Point{ wndattr.width, wndattr.height };
		
        auto name = candidates.empty() ? std::string() : std::move(candidates.front());
        if (name.size() > sizeof(w.Name) - 1) {
            name.resize(sizeof(w.Name) - 1);
        }
        std::transform(name.begin(), name.end(), std::begin(w.Name), ::tolower);
        wnd.push_back(w);
    }

    std::vector<Window> GetWindows()
    {
        auto* display = XOpenDisplay(NULL);
        Atom a = XInternAtom(display, "_NET_CLIENT_LIST", true);
        Atom actualType;
        int format;
        unsigned long numItems, bytesAfter;
        unsigned char* data = 0;
        int status = XGetWindowProperty(display,
                                        XDefaultRootWindow(display),
                                        a,
                                        0L,
                                        (~0L),
                                        false,
                                        AnyPropertyType,
                                        &actualType,
                                        &format,
                                        &numItems,
                                        &bytesAfter,
                                        &data);
        std::vector<Window> ret;
        if(status >= Success && numItems) {
            auto array = (XID*)data;
            for(decltype(numItems) k = 0; k < numItems; k++) {
                auto w = array[k];
                AddWindow(display, w, ret);
            }
            XFree(data);
        }
        XCloseDisplay(display);
        return ret;
    }
}
}
