using System;
using System.Collections.Generic;
using System.Text;

namespace Core
{
    public delegate void DispatcherTimerTickEventHandler(IDispatcherTimer timer);

    public interface IDispatcherTimer : IDisposable
    {
        event DispatcherTimerTickEventHandler Tick;
        TimeSpan Interval { get; set; }
        void Start();
        void Stop();
    }
}
