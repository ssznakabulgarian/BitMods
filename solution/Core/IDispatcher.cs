using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace Core
{
    public interface IDispatcher
    {
        IAsyncResult BeginInvoke(Delegate method, params object[] args);
        object EndInvoke(IAsyncResult result);
        T EndInvokeTyped<T>(IAsyncResult result);
        Task<T> DoFunc<T>(Func<T> func);
        Task DoAction(Action func);
        Task DoAction<T>(Action<T> func, T argument);
        void ScheduleInvoke(TimeSpan span, Delegate method, params object[] args);
        IDispatcherTimer CreateTimer();
    }

}
