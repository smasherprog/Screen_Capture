using System;
using System.Diagnostics;
using System.Drawing;
using System.Threading;

namespace screen_capture_lite_example_csharp
{
    public class Program
    {
        private static void WriteLine(ref SL.Screen_Capture.Monitor p)
        {
            Console.WriteLine($"Id = {p.Id} Index = {p.Index} Height = {p.Height} Width = {p.Width} OffsetX = {p.OffsetX} OffsetY= {p.OffsetY} Name= {p.Name}");
        }
        private static void WriteLine(ref SL.Screen_Capture.Image p)
        {
            Console.WriteLine($"BytesToNextRow = {p.BytesToNextRow} isContiguous = {p.isContiguous} Bounds.bottom = {p.Bounds.bottom} Bounds.left = {p.Bounds.left} Bounds.right = {p.Bounds.right} Bounds.top = {p.Bounds.top}");
        }
        static int onNewFramecounter = 0;
        static DateTime onNewFramestart = DateTime.Now;
        public static SL.Screen_Capture.CaptureConfiguration.ScreenCaptureManager createframegrabber()
        {
            onNewFramecounter = 0;
            onNewFramestart = DateTime.Now;
            return SL.Screen_Capture.CaptureConfiguration.CreateCaptureConfiguration(() =>
            {
                var mons = SL.Screen_Capture.GetMonitors();
                Console.WriteLine("Library is requesting the list of monitors to capture!");
                for (int i = 0; i < mons.Length; ++i)
                {
                    WriteLine(ref mons[i]);
                }
                return mons;
            }).onNewFrame((ref SL.Screen_Capture.Image img, ref SL.Screen_Capture.Monitor monitor) =>
            {
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onNewFrame.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine(ref img);
                //WriteLine(ref monitor); 
                if ((DateTime.Now - onNewFramestart).TotalMilliseconds > 1000)
                {
                    Console.WriteLine("onNewFrame fps" + onNewFramecounter);
                    onNewFramestart = DateTime.Now;
                    onNewFramecounter = 0;
                }
                Interlocked.Increment(ref onNewFramecounter);

            }).onFrameChanged((ref SL.Screen_Capture.Image img, ref SL.Screen_Capture.Monitor monitor) =>
            { 
                //var newBitmap = new Bitmap(img.Bounds.right - img.Bounds.left, img.Bounds.bottom - img.Bounds.top, img.BytesToNextRow, System.Drawing.Imaging.PixelFormat.Format32bppRgb, img.Data);
                //newBitmap.Save($"{onNewFramecounter++}onFrameChanged.jpg", System.Drawing.Imaging.ImageFormat.Jpeg);
                //WriteLine(ref img);
                //WriteLine(ref monitor); 
            }).start_capturing();
        }

        public static void Main(string[] args)
        {
            Console.WriteLine("Starting Capture Demo/Test");
            var goodmonitors = SL.Screen_Capture.GetMonitors();
            for (int i = 0; i < goodmonitors.Length; ++i)
            {
                WriteLine(ref goodmonitors[i]);
                Debug.Assert(SL.Screen_Capture.isMonitorInsideBounds(goodmonitors, goodmonitors[i]));
            }

            var badmonitors = SL.Screen_Capture.GetMonitors();
            for (int i = 0; i < badmonitors.Length; ++i)
            {
                badmonitors[i].Height += 1;
                WriteLine(ref badmonitors[i]);
                Debug.Assert(!SL.Screen_Capture.isMonitorInsideBounds(goodmonitors, badmonitors[i]));
            }
            for (int i = 0; i < badmonitors.Length; ++i)
            {
                badmonitors[i].Width += 1;
                WriteLine(ref badmonitors[i]);
                Debug.Assert(!SL.Screen_Capture.isMonitorInsideBounds(goodmonitors, badmonitors[i]));
            }

            Console.WriteLine("Running display capturing for 10 seconds");
            using (var framgrabber = createframegrabber())
            {
                System.Threading.Thread.Sleep(10 * 1000); 
            }

            Console.WriteLine("Running display capturing for 1 seconds");
            using (var framgrabber = createframegrabber())
            {
                System.Threading.Thread.Sleep(1 * 1000);
                Console.WriteLine("Pausing for 10 seconds.");
                framgrabber.pause();
                var counti = 0;
                while (counti++ < 10)
                {
                    Debug.Assert(framgrabber.isPaused());
                    Console.Write(" . ");
                    System.Threading.Thread.Sleep(1 * 1000);
                }
                Console.WriteLine("Resuming  . . . for 5 seconds");
                framgrabber.resume();
                System.Threading.Thread.Sleep(5 * 1000);
            }
        }
    }
}
