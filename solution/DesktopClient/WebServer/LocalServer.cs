using Core;
using EmbedIO;
using EmbedIO.WebApi;
using EmbedIO.WebSockets;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebCore;

namespace DesktopClient.WebServer
{
    class LocalServer
    {
        public static BaseServer Server;
        public static void Start(IDispatcher dispatcher, Action<string> callback)
        {
            Server = new BaseServer(dispatcher, new CommContextFactory(), "http://localhost:9999/");
            Server.Server.WithWebApi("/local", m => m.WithController(() => new LocalController(dispatcher)));
            Server.Start(callback);
        }

        public static void Stop()
        {
            Server.Stop();
        }

        private class CommContextFactory : ICommContextFactory
        {
            public CommContext Create(WebSocketModuleExt module, IWebSocketContext context)
            {
                return new LocalCommContext(module, context);
            }
        }
    }
}
