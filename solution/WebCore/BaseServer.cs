using Core;
using EmbedIO;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using WebCore.Messages;

namespace WebCore
{
    public class BaseServer
    {
        public readonly IDispatcher Dispatcher;
        public readonly ICommContextFactory CommContextFactory;
        public readonly WebServer Server;
        private CancellationTokenSource mCancellationTokenSource;
        public static readonly Dictionary<string, Type> MessageNameToType = new();

        public static void RegisterAssemblyMessages(Assembly assembly)
        {
            foreach(Type type in assembly.GetTypes())
            {
                Type current = type;
                while(current != null)
                {
                    if(current == typeof(CommMessage))
                    {
                        MessageNameToType[type.Name] = type;
                        break;
                    }
                    current = current.BaseType;
                }
            }
        }

        public BaseServer(IDispatcher dispatcher, ICommContextFactory commContextFactory, string prefix)
        {
            RegisterAssemblyMessages(GetType().Assembly);
            Dispatcher = dispatcher;
            CommContextFactory = commContextFactory;
            Server = new WebServer(o => o.WithUrlPrefix(prefix).WithMode(HttpListenerMode.EmbedIO));
            Server.WithLocalSessionManager();
        }

        public void Start(Action<string> callback) {
            bool callbackCalled = false;
            Server.WithModule(new WebSocketCommModule(Dispatcher, "/socket", CommContextFactory));
            Server.WithStaticFolder("/", "C:/Users/gbald/Downloads/BitMods/web", false);
            Server.StateChanged += (s, e) =>
            {
                if (callbackCalled) return;
                if (e.NewState == WebServerState.Listening)
                {
                    callbackCalled = true;
                    Dispatcher.DoAction(callback, null);
                }
                else if (e.NewState == WebServerState.Stopped)
                {
                    callbackCalled = true;
                    Dispatcher.DoAction(callback, "Failed to start web server!");
                }
            };

            mCancellationTokenSource = new CancellationTokenSource();

            _ = Server.RunAsync(mCancellationTokenSource.Token);
        }

        public void Stop()
        {
            if (mCancellationTokenSource != null)
            {
                mCancellationTokenSource.Cancel();
            }
        }
    }
}
