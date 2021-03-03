using Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DesktopClient
{
    class DispatcherImpl : IDispatcher
    {
        private readonly Form mForm;

        public DispatcherImpl(Form form)
        {
            mForm = form;
        }

        public IAsyncResult BeginInvoke(Delegate method, params object[] args)
        {
            return mForm.BeginInvoke(method, args);
        }

        public object EndInvoke(IAsyncResult result)
        {
            return mForm.EndInvoke(result);
        }

        public T EndInvokeTyped<T>(IAsyncResult result)
        {
            return (T)EndInvoke(result);
        }
        public Task<T> DoFunc<T>(Func<T> func)
        {
            return Task<T>.Factory.FromAsync(
                BeginInvoke(func), EndInvokeTyped<T>);
        }
        public Task DoAction(Action action)
        {
            return Task.Factory.FromAsync(
                BeginInvoke(action), EndInvoke);
        }
        public Task DoAction<T>(Action<T> action, T argument)
        {
            return Task.Factory.FromAsync(
                BeginInvoke(action, argument), EndInvoke);
        }

        public IDispatcherTimer CreateTimer()
        {
            return new DispatchertimerImpl();
        }

        public void ScheduleInvoke(TimeSpan span, Delegate method, params object[] args)
        {
            Timer timer = new Timer();
            timer.Interval = (int)span.TotalMilliseconds;
            timer.Tick += (object sender, EventArgs e) => { timer.Stop();  timer.Dispose(); method.DynamicInvoke(args); };
            timer.Start();
        }

        private class DispatchertimerImpl : IDispatcherTimer
        {
            private readonly Timer mTimer = new Timer();

            public DispatchertimerImpl()
            {
                mTimer.Tick += MTimer_Tick;
            }

            private void MTimer_Tick(object sender, EventArgs e)
            {
                if (Tick != null) Tick(this);
            }

            public TimeSpan Interval
            {
                get
                {
                    return TimeSpan.FromMilliseconds(mTimer.Interval);
                }
                set
                {
                    mTimer.Interval = (int)value.TotalMilliseconds;
                }
            }

            public event DispatcherTimerTickEventHandler Tick;

            public void Start()
            {
                mTimer.Start();
            }

            public void Stop()
            {
                mTimer.Stop();
            }

            public void Dispose()
            {
                mTimer.Dispose();
            }
        }
    }
}
