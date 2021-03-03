using EmbedIO.WebSockets;
using Swan.Formatters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebCore.Messages;

namespace WebCore
{
    public abstract class WebSocketModuleExt : WebSocketModule
    {
        public WebSocketModuleExt(string urlPath) : base(urlPath, true)
        {
        }

        public void Send(IWebSocketContext context, CommMessage message)
        {
            string type = message.GetType().Name;
            string json = Json.Serialize(message);
            _ = SendAsync(context, type + ":" + json);
        }
    }
}
