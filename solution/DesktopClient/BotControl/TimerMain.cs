using ControlCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DesktopClient.BotControl
{
    class TimerMain : ControlCore.ITimer
    {
        private readonly BotMain mBot;
        private string mId;
        private long mTime;

        public TimerMain(BotMain bot)
        {
            mBot = bot;
        }

        public string Type { get { return "Timer";  } }

        public string GlobalId { get { return "238747823874"; } }

        public string Id { get { return mId; } set { mId = value;  } }

        public event ModStateChangedEventHandler OnModStateChanged;

        public long GetTime()
        {
            return mTime;
        }

        public void Reset()
        {
            mBot.Send("RSET");
        }

        public void SetTime(long time)
        {
            mTime = time;
            if (OnModStateChanged != null) OnModStateChanged(mBot, this);
        }
    }
}
