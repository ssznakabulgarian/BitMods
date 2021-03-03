using ControlCore;
using Core;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DesktopClient.BotControl
{
    delegate void RunningChangedEventHandler(BotMain sender, bool running);

    class BotMain : IBot, IDisposable
    {
        private static readonly TimeSpan RETRY_CONNECTION_INTERVAL = TimeSpan.FromSeconds(3);

        enum ProtocolState
        {
            Idle,
            InitSent,
            Connected,
            Finished
        }

        private readonly object mSync = new object();
        public readonly string Id;
        private readonly IDispatcher mDispatcher;
        private readonly IDispatcherTimer mProtocolTimeout;
        private bool mIsAvailable = false;
        private string mLastAvalabilityError = null;
        private bool mRunning = false;
        private IBotConnection mConnection;
        private ProtocolState mState;
        private TimerMain mTimerMain;

        public BotMain(IDispatcher dispatcher, string id)
        {
            if (dispatcher == null) throw new ArgumentNullException();
            Id = id;
            mDispatcher = dispatcher;
            mProtocolTimeout = mDispatcher.CreateTimer();
            mProtocolTimeout.Tick += MProtocolTimeout_Tick;
        }

        public void Dispose()
        {
            mProtocolTimeout.Dispose();
        }

        public event RunningChangedEventHandler RunningChanged;
        public bool Running { get { return mRunning; } }

        public void SetConnection(IBotConnection connection)
        {
            if(mRunning) throw new InvalidOperationException();
            if (mConnection != null)
            {
                mConnection.StateChanged -= MConnection_StateChanged;
                mConnection.Message -= MConnection_Message;
            }
            mConnection = connection;
            if(mConnection != null)
            {
                mConnection.StateChanged += MConnection_StateChanged;
                mConnection.Message += MConnection_Message;
            }
        }

        public IBotConnection GetConnection()
        {
            return mConnection;
        }

        private delegate void ReconnectEvnetHandler();

        private void Reconnect()
        {
            if(mRunning && ((mConnection.State == BotConnectionState.Stopped) || (mConnection.State == BotConnectionState.Failed))) {
                mState = ProtocolState.Idle;
                mConnection.Start();
                UpdateTimeout();
            }
        }

        private void MConnection_StateChanged(IBotConnection sender, BotConnectionState state)
        {
            if (!mRunning) return;
            if ((state == BotConnectionState.Stopped) || (state == BotConnectionState.Failed)) {
                ProtocolError((mConnection.LastError != null) ? mConnection.LastError : "Connection failed.");
            }
            else if(state == BotConnectionState.Started)
            {
                mConnection.SendMessage(Encoding.ASCII.GetBytes("INIT"));
                mState = ProtocolState.InitSent;
                UpdateTimeout();
            }
        }

        private void MConnection_Message(IBotConnection sender, byte[] message)
        {
            switch(mState)
            {
                case ProtocolState.InitSent:
                    if(Encoding.ASCII.GetString(message) == "REDY")
                    {
                        mState = ProtocolState.Connected;
                        UpdateTimeout();
                        SetAvailable(true);
                    }
                    else
                    {
                        ProtocolError("Invalid response from bot.");
                    }
                    break;
                case ProtocolState.Connected:
                    string text = Encoding.ASCII.GetString(message);
                    int time;
                    if(int.TryParse(text, out time))
                    {
                        if (mTimerMain != null) mTimerMain.SetTime(time);
                    }
                    else
                    {
                        ProtocolError("Invalid message from bot.");
                    }
                    break;
                default:
                    ProtocolError("Unexpected response from bot.");
                    break;
            }
        }

        private void UpdateTimeout()
        {
            TimeSpan timeout = TimeSpan.Zero;
            switch (mState)
            {
                case ProtocolState.Idle: timeout = TimeSpan.FromSeconds(5); break;
                case ProtocolState.InitSent: timeout = TimeSpan.FromSeconds(5); break;
            }
            mProtocolTimeout.Stop();
            if (timeout != TimeSpan.Zero)
            {
                mProtocolTimeout.Interval = timeout;
                mProtocolTimeout.Start();
            }
        }

        private void MProtocolTimeout_Tick(IDispatcherTimer timer)
        {
            mState = ProtocolState.Finished;
            ProtocolError("Communication timeout.");
        }

        private void ProtocolError(string error)
        {
            if (!mRunning) return;
            mLastAvalabilityError = error;
            mState = ProtocolState.Finished;
            UpdateTimeout();
            mConnection.BeginStop();
            mDispatcher.ScheduleInvoke(RETRY_CONNECTION_INTERVAL, new ReconnectEvnetHandler(Reconnect));
            SetAvailable(false);
        }

        public void Start()
        {
            if(!mRunning)
            {
                mRunning = true;
                Reconnect();
                if (RunningChanged != null) RunningChanged(this, mRunning);
            }
        }

        public void Stop()
        {
            if(mRunning)
            {
                mRunning = false;
                mConnection.BeginStop();
                UpdateTimeout();
                if (RunningChanged != null) RunningChanged(this, mRunning);
            }
        }

        public void Send(string message)
        {
            if (mConnection != null)
            {
                mConnection.SendMessage(Encoding.ASCII.GetBytes(message));
            }
        }

        private void SetAvailable(bool isAvailable)
        {
            if (mIsAvailable != isAvailable)
            {
                mIsAvailable = isAvailable;
                if(mIsAvailable)
                {
                    mTimerMain = new TimerMain(this);
                    if (OnModAdded != null) OnModAdded(this, mTimerMain);
                }
                else
                {
                    mTimerMain = null;
                    if (OnModRemoved != null) OnModRemoved(this, mTimerMain);
                }
                if (IsAvailableChanged != null) IsAvailableChanged(this, isAvailable);
            }
        }

        public bool IsAvailable
        {
            get
            {
                lock (mSync)
                {
                    return mIsAvailable;
                }
            }
        }

        public string GetLastAvalabilityError()
        {
            return mLastAvalabilityError;
        }

        public event IsAvailableChangedEventHandler IsAvailableChanged;

        public event ModAddedEventHandler OnModAdded;
        public event ModRemovedEventHandler OnModRemoved;
        public event ModIdChangedEventHandler OnModIdChanged;
        public event ProgramStateChangedEventHandler OnProgramStateChanged;
        public event ProgramOutputEventHandler OnProgramOutput;

        public string GetLastCompileError()
        {
            throw new NotImplementedException();
        }

        public IMod GetMod(string id)
        {
            if ((mTimerMain != null) && (mTimerMain.Id == id)) return mTimerMain;
            return null;
        }

        public IMod[] GetMods()
        {
            if (mTimerMain != null) return new IMod[] { mTimerMain };
            return new IMod[0];
        }

        public string GetProgram()
        {
            throw new NotImplementedException();
        }

        public ProgramState GetProgramState()
        {
            throw new NotImplementedException();
        }

        public void SendProgramInput(string input)
        {
            throw new NotImplementedException();
        }

        public void SetProgram(string code)
        {
            throw new NotImplementedException();
        }

        public void StartProgram()
        {
            throw new NotImplementedException();
        }

        public void StopProgram()
        {
            throw new NotImplementedException();
        }
    }
}
