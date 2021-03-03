using Core;
using EmbedIO.WebSockets;
using Swan.Formatters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using WebCore.Messages;

namespace WebCore
{
    class WebSocketCommModule : WebSocketModuleExt
    {
        public readonly IDispatcher Dispatcher;
        private readonly ICommContextFactory mCommContextFactory;
        private readonly Dictionary<IWebSocketContext, CommContext> mContextMap = new Dictionary<IWebSocketContext, CommContext>();

        public WebSocketCommModule(IDispatcher dispatcher, string urlPath, ICommContextFactory commContextFactory) : base(urlPath)
        {
            Dispatcher = dispatcher;
            mCommContextFactory = commContextFactory;
            AddProtocol("comm");
        }

        protected override Task OnClientConnectedAsync(IWebSocketContext context)
        {
            return Dispatcher.DoAction(() =>
            {
                CommContext commContext = mCommContextFactory.Create(this, context);
                mContextMap[context] = commContext;
            });
        }

        protected override Task OnClientDisconnectedAsync(IWebSocketContext context)
        {
            return Dispatcher.DoAction(() =>
            {
                mContextMap.Remove(context);
            });
        }

        protected override Task OnMessageReceivedAsync(IWebSocketContext context, byte[] buffer, IWebSocketReceiveResult result)
        {
            return Dispatcher.DoAction(() =>
            {
                CommContext commContext;
                if (!mContextMap.TryGetValue(context, out commContext))
                {
                    commContext = null;
                }
                if (commContext != null)
                {
                    string json = Encoding.UTF8.GetString(buffer);
                    int index = json.IndexOf(":");
                    if(index > 0)
                    {
                        string type = json.Substring(0, index);
                        json = json.Substring(index + 1);
                        Type t;
                        if(BaseServer.MessageNameToType.TryGetValue(type, out t))
                        {
                            CommMessage message = Json.Deserialize(json, t) as CommMessage;
                            if(message != null)
                            {
                                message.Handle(commContext);
                            }
                        }
                    }
                }
            });
        }
    }
}
