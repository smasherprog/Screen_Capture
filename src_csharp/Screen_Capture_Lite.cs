using System;
using System.Runtime.InteropServices;

namespace SL
{
    public static class Screen_Capture
    {
        [StructLayout(LayoutKind.Sequential)]
        public struct Point
        {
            public int x;
            public int y;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct MousePoint
        {
            public Point Position;
            public Point HotSpot;
        };

        [StructLayout(LayoutKind.Sequential)]
        public struct Window
        {
            public IntPtr Handle;
            public Point Position;
            public Point Size;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string Name;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct Monitor
        {
            public int Id;
            public int Index;
            public int Adapter;
            public int Height;
            public int Width;
            public int OriginalHeight;
            public int OriginalWidth;
            // Offsets are the number of pixels that a monitor can be from the origin. For example, users can shuffle their
            // monitors around so this affects their offset.
            public int OffsetX;
            public int OffsetY;
            public int OriginalOffsetX;
            public int OriginalOffsetY;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string Name;
            public float Scaling;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct ImageRect
        {
            public int left;
            public int top;
            public int right;
            public int bottom;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct Image
        {
            public ImageRect Bounds;
            public int BytesToNextRow;
            [MarshalAs(UnmanagedType.I1)]
            public bool isContiguous;
            // alpha is always unused and might contain garbage
            public IntPtr Data;
        }
        [StructLayout(LayoutKind.Sequential)]
        public struct ImageBGRA
        {
            public char B;
            public char G;
            public char R;
            public char A;
        }

        public delegate Monitor[] MonitorCallback();
        public delegate void ScreenCaptureCallback(ref Image img, ref Monitor monitor);

        public class CaptureConfiguration : IDisposable
        {
            public class CaptureData<MonitorCallback, ScreenCaptureCallback>
            {
                public MonitorCallback getThingsToWatch;
                public ScreenCaptureCallback OnFrameChanged;
                public ScreenCaptureCallback OnNewFrame;
                public int FrameTimerInMS = 100;
            };
            public class ScreenCaptureManager : IDisposable
            {

                private CaptureData<NativeFunctions.MonitorCallback, ScreenCaptureCallback> ScreenCaptureData;
                private bool disposedValue;
                private IntPtr ScreenCapturePtr = IntPtr.Zero;

                public ScreenCaptureManager(IntPtr p, CaptureData<NativeFunctions.MonitorCallback, ScreenCaptureCallback> d)
                {
                    ScreenCapturePtr = p;
                    ScreenCaptureData = d;
                }

                public void pause()
                {
                    NativeFunctions.pause(ScreenCapturePtr);
                }

                public void resume()
                {
                    NativeFunctions.resume(ScreenCapturePtr);
                }

                public bool isPaused()
                {
                    return NativeFunctions.isPaused(ScreenCapturePtr);
                }

                public void setFrameChangeInterval(TimeSpan timeSpan)
                {
                    NativeFunctions.setFrameChangeInterval(ScreenCapturePtr, (int)timeSpan.TotalMilliseconds);
                }

                protected virtual void Dispose(bool disposing)
                {
                    if (!disposedValue)
                    {
                        NativeFunctions.FreeIScreenCaptureManagerWrapper(ScreenCapturePtr);
                        ScreenCapturePtr = IntPtr.Zero;
                        disposedValue = true;
                    }
                }

                ~ScreenCaptureManager()
                {
                    // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                    Dispose(disposing: false);
                }

                public void Dispose()
                {
                    // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                    Dispose(disposing: true);
                    GC.SuppressFinalize(this);
                }
            }

            private CaptureData<NativeFunctions.MonitorCallback, ScreenCaptureCallback> Impl_;
            private bool disposedValue;
            private IntPtr CaptureConfigurationPtr = IntPtr.Zero;
            public static CaptureConfiguration CreateCaptureConfiguration(MonitorCallback monitorstocapture)
            {
                NativeFunctions.MonitorCallback newcb = (IntPtr monitorbuffer, int monitorbuffersize) =>
                {
                    var m = monitorstocapture();
                    var monitorstocopy = m.Length > monitorbuffersize ? monitorbuffersize : m.Length;
                    for (var i = 0; i < monitorstocopy; i++)
                    {
                        Marshal.StructureToPtr(m[i], monitorbuffer, false);
                        var size = Marshal.SizeOf(typeof(Monitor));
                        monitorbuffer = IntPtr.Add(monitorbuffer, size);
                    }
                    return m.Length;
                };
                return new CaptureConfiguration
                {
                    Impl_ = new CaptureData<NativeFunctions.MonitorCallback, ScreenCaptureCallback>
                    {
                        getThingsToWatch = newcb
                    },
                    CaptureConfigurationPtr = NativeFunctions.CreateCaptureConfiguration(newcb)
                };
            }

            public CaptureConfiguration onNewFrame(ScreenCaptureCallback cb)
            {
                Impl_.OnNewFrame = cb;
                NativeFunctions.onNewFrame(CaptureConfigurationPtr, cb);
                return this;
            }

            public CaptureConfiguration onFrameChanged(ScreenCaptureCallback cb)
            {
                Impl_.OnFrameChanged = cb;
                NativeFunctions.onFrameChanged(CaptureConfigurationPtr, cb);
                return this;
            }

            public ScreenCaptureManager start_capturing()
            {
                var r = new ScreenCaptureManager(NativeFunctions.start_capturing(CaptureConfigurationPtr), Impl_);
                CaptureConfigurationPtr = IntPtr.Zero;
                return r;
            }

            protected virtual void Dispose(bool disposing)
            {
                if (!disposedValue)
                {
                    if (disposing)
                    {
                        // TODO: dispose managed state (managed objects)
                    }

                    NativeFunctions.FreeCaptureConfiguration(CaptureConfigurationPtr);
                    CaptureConfigurationPtr = IntPtr.Zero;
                    disposedValue = true;
                }
            }

            ~CaptureConfiguration()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: false);
            }

            public void Dispose()
            {
                // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
                Dispose(disposing: true);
                GC.SuppressFinalize(this);
            }
        }

        private static int GetMonitorsAllocationSize = 16;
        public static Monitor[] GetMonitors()
        {
            var monitorsizeguess = GetMonitorsAllocationSize;
            var size = Marshal.SizeOf(typeof(Monitor));
            var unmanagedArray = Marshal.AllocHGlobal(monitorsizeguess * size);
            var sizeneeded = NativeFunctions.GetMonitors(unmanagedArray, monitorsizeguess);
            if (monitorsizeguess < sizeneeded)
            {
                monitorsizeguess = sizeneeded;
                GetMonitorsAllocationSize = Math.Max(sizeneeded, GetMonitorsAllocationSize);
                Marshal.FreeHGlobal(unmanagedArray);
                unmanagedArray = Marshal.AllocHGlobal(monitorsizeguess * size);
                sizeneeded = NativeFunctions.GetMonitors(unmanagedArray, monitorsizeguess);
            }

            monitorsizeguess = Math.Min(sizeneeded, monitorsizeguess);
            var copyunmanagedArray = unmanagedArray;
            var mangagedArray = new Monitor[monitorsizeguess];
            for (int i = 0; i < monitorsizeguess; i++)
            {
                mangagedArray[i] = Marshal.PtrToStructure<Monitor>(unmanagedArray);
                unmanagedArray = IntPtr.Add(unmanagedArray, size);
            }
            Marshal.FreeHGlobal(copyunmanagedArray);
            return mangagedArray;
        }
        public static bool isMonitorInsideBounds(Monitor[] monitors, Monitor monitor)
        {
            return NativeFunctions.isMonitorInsideBounds(monitors, monitors.Length, monitor);
        }

        public static class NativeFunctions
        {
            public delegate int MonitorCallback(IntPtr monitorbuffer, int monitorbuffersize);

            [DllImport("screen_capture_lite_shared")]
            public static extern int GetMonitors(IntPtr monitors, int monitors_size);
            [DllImport("screen_capture_lite_shared")]
            [return: MarshalAs(UnmanagedType.I1)]
            public static extern bool isMonitorInsideBounds(Monitor[] monitors, int monitorsize, Monitor monitor);
            [DllImport("screen_capture_lite_shared")]
            public static extern IntPtr CreateCaptureConfiguration(MonitorCallback monitorCallback);
            [DllImport("screen_capture_lite_shared")]
            public static extern void FreeCaptureConfiguration(IntPtr ptr);
            [DllImport("screen_capture_lite_shared")]
            public static extern void pause(IntPtr ptr);
            [DllImport("screen_capture_lite_shared")]
            [return: MarshalAs(UnmanagedType.I1)]
            public static extern bool isPaused(IntPtr ptr);
            [DllImport("screen_capture_lite_shared")]
            public static extern bool resume(IntPtr ptr);
            [DllImport("screen_capture_lite_shared")]
            public static extern void setFrameChangeInterval(IntPtr ptr, int milliseconds);
            [DllImport("screen_capture_lite_shared")]
            public static extern void onNewFrame(IntPtr ptr, ScreenCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared")]
            public static extern void onFrameChanged(IntPtr ptr, ScreenCaptureCallback monitorCallback);
            [DllImport("screen_capture_lite_shared")]
            //this function will free the input pointer calling FreeCaptureConfiguration internally since its no longer needed. This makes it easier to reason about the c# code.
            public static extern IntPtr start_capturing(IntPtr ptr);
            [DllImport("screen_capture_lite_shared")]
            public static extern void FreeIScreenCaptureManagerWrapper(IntPtr ptr);
        }
    }
}