using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DesktopClient.BotControl
{
    delegate void BotConnectionMessageEventHandler(IBotConnection sender, byte[] message);
    delegate void BotConnectionStateChangedEventHandler(IBotConnection sender, BotConnectionState state);

    public enum BotConnectionState
    {
        Starting,
        Started,
        Failed,
        Stopped
    }

    interface IBotConnection
    {
        event BotConnectionStateChangedEventHandler StateChanged;
        event BotConnectionMessageEventHandler Message;
        BotConnectionState State { get; }
        void Start();
        bool BeginStop();
        void WaitForStop();
        void SendMessage(byte[] message);
        string LastError { get; }
        string LastErrorDetails { get; }
    }
}
