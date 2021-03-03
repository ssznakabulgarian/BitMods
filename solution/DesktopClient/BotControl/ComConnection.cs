using Core;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Threading;

namespace DesktopClient.BotControl
{
    class ComConnection : IBotConnection
    {
        private readonly object mSync = new object();
        private readonly IDispatcher mDispatcher;
        private string mPortName = "";
        private BotConnectionState mState = BotConnectionState.Stopped;

        private Queue<byte[]> mWriteQueue = new Queue<byte[]>();

        private SerialPort mSerialPort;
        private string mLastError;
        private string mLastErrorDetails;
        private volatile bool mStopping = false;

        private Thread mReadThread;
        private Thread mWriteThread;

        private AutoResetEvent mReadEvent = new AutoResetEvent(false);
        private AutoResetEvent mWriteEvent = new AutoResetEvent(false);

        public ComConnection(IDispatcher dispatcher)
        {
            mDispatcher = dispatcher;
        }

        public event BotConnectionMessageEventHandler Message;
        public event BotConnectionStateChangedEventHandler StateChanged;

        public BotConnectionState State { get { return mState; } }

        public void SendMessage(byte[] message)
        {
            if (message == null) throw new ArgumentNullException();
            lock (mSync)
            {
                if ((mState != BotConnectionState.Starting) && (mState != BotConnectionState.Started)) return;
                mWriteQueue.Enqueue(message);
                mWriteEvent.Set();
            }
        }

        public string LastError { get { return mLastError; } }
        public string LastErrorDetails { get { return mLastErrorDetails; } }

        public void Start()
        {
            lock (mSync)
            {
                if((mState != BotConnectionState.Stopped) && (mState != BotConnectionState.Failed)) throw new InvalidOperationException();

                mLastError = null;
                mLastErrorDetails = null;
                mWriteQueue.Clear();
                mState = BotConnectionState.Starting;
                mStopping = false;
                mReadThread = new Thread(ReadWorker);
                mReadThread.Start();
                mWriteThread = new Thread(WriteWorker);
                mWriteThread.Start();
            }
            if (StateChanged != null) StateChanged(this, mState);
        }

        public bool BeginStop()
        {
            lock (mSync)
            {
                if ((mState != BotConnectionState.Starting) && (mState != BotConnectionState.Started)) return false;

                mStopping = true;
                mReadEvent.Set();
                mWriteEvent.Set();
            }
            return true;
        }

        public void WaitForStop()
        {
            Thread readThread;
            Thread writeThread;
            lock (mSync)
            {
                readThread = mReadThread;
                writeThread = mWriteThread;
            }
            if(readThread != null)
            {
                readThread.Join();
            }
            if (writeThread != null)
            {
                writeThread.Join();
            }
        }

        public void SetPortName(string portName)
        {
            if (portName == null) throw new ArgumentNullException();
            lock (mSync)
            {
                if ((mState != BotConnectionState.Stopped) && (mState != BotConnectionState.Failed)) throw new InvalidOperationException();
                mPortName = portName;
            }
        }

        public string GetPortName()
        {
            return mPortName;
        }

        private delegate void CallStateChangeEventHandler(BotConnectionState state);

        private void CallStateChange(BotConnectionState state)
        {
            lock (mSync)
            {
                mState = state;
            }
            if (StateChanged != null) StateChanged(this, state);
        }

        private delegate void CallMessageEventHandler(byte[] message);

        private void CallMessage(byte[] message)
        {
            if (Message != null) Message(this, message);
        }

        private void ThreadFinished(bool read, bool write, string error, string details)
        {
            lock (mSync)
            {
                if (error != null)
                {
                    mLastError = error;
                    mLastErrorDetails = details;
                }
                if (read) mReadThread = null;
                if (write) mWriteThread = null;
                if ((mReadThread == null) && (mWriteThread == null)) {
                    BotConnectionState state = (mLastError != null) ? BotConnectionState.Failed : BotConnectionState.Stopped;
                    mDispatcher.BeginInvoke(new CallStateChangeEventHandler(CallStateChange), state);
                }
                else
                {
                    mStopping = true;
                    if (mReadThread != null) mReadEvent.Set();
                    if (mWriteThread != null) mWriteEvent.Set();
                }
            }
        }

        private const int MESSAGE_LENGTH = 4;
        private byte[] mIncommingMessage = new byte[MESSAGE_LENGTH];
        private int mIncommingMessageLength = 0;

        private void ProcessIncomming(byte[] buffer, int from, int to)
        {
            for(int i = from; i < to; i++)
            {
                mIncommingMessage[mIncommingMessageLength] = buffer[i];
                mIncommingMessageLength++;
                if(mIncommingMessageLength == mIncommingMessage.Length)
                {
                    mDispatcher.BeginInvoke(new CallMessageEventHandler(CallMessage), mIncommingMessage);
                    mIncommingMessage = new byte[MESSAGE_LENGTH];
                    mIncommingMessageLength = 0;
                }
            }
        }

        private void ReadWorker()
        {
            byte[] buffer = new byte[1024];

            mSerialPort = new SerialPort();
            mSerialPort.PortName = mPortName;
            mSerialPort.BaudRate = 9600;
            mSerialPort.DataBits = 8;
            mSerialPort.Parity = Parity.None;
            mSerialPort.StopBits = StopBits.One;
            mSerialPort.Handshake = Handshake.RequestToSend;
            mSerialPort.DtrEnable = true;
            mSerialPort.DataReceived += SerialPort_DataReceived;
            try
            {
                mSerialPort.Open();
            }
            catch(Exception exception)
            {
                ThreadFinished(true, false, "Failed to open serial port: " + mPortName, exception.ToString());
                return;
            }
            mDispatcher.BeginInvoke(new CallStateChangeEventHandler(CallStateChange), BotConnectionState.Started);
            try
            {
                try
                {
                    while (!mStopping)
                    {
                        while (!mStopping && (mSerialPort.BytesToRead > 0))
                        {
                            int read = mSerialPort.Read(buffer, 0, buffer.Length);
                            ProcessIncomming(buffer, 0, read);
                        }
                        mReadEvent.WaitOne();
                    }
                }
                finally
                {
                    mSerialPort.Close();
                }
            }
            catch (Exception exception)
            {
                ThreadFinished(true, false, "Communication failed on port: " + mPortName, exception.ToString());
                return;
            }

            ThreadFinished(true, false, null, null);
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            mReadEvent.Set();
        }

        private void WriteWorker()
        {
            while(!mStopping)
            {
                byte[] message = null;
                lock(mSync)
                {
                    if(mWriteQueue.Count > 0)
                    {
                        message = mWriteQueue.Dequeue();
                    }
                }
                if(message != null)
                {
                    try
                    {
                        mSerialPort.Write(message, 0, message.Length);
                    }
                    catch (Exception exception)
                    {
                        ThreadFinished(false, true, "Communication failed on port: " + mPortName, exception.ToString());
                        return;
                    }
                }
                else
                {
                    mWriteEvent.WaitOne();
                }
            }
            ThreadFinished(false, true, null, null);
        }
    }
}
