using ControlCore;
using EmbedIO.WebSockets;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebCore.Messages;

namespace WebCore
{
    public abstract class CommContext
    {
        public readonly WebSocketModuleExt Module;
        public readonly IWebSocketContext Context;

        public CommContext(WebSocketModuleExt module, IWebSocketContext context)
        {
            Module = module;
            Context = context;
        }

        public abstract IBot LockBot(string botId);

        public void Send(CommMessage message)
        {
            Module.Send(Context, message);
        }
    }
}
