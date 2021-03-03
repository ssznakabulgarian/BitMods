using ControlCore;
using DesktopClient.BotControl;
using EmbedIO.WebSockets;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebCore;

namespace DesktopClient.WebServer
{
    internal class LocalCommContext : CommContext
    {
        public LocalCommContext(WebSocketModuleExt module, IWebSocketContext context) : base(module, context)
        {
        }

        public override IBot LockBot(string botId)
        {
            foreach(BotMain bot in BotSet.IdleBots)
            {
                if(bot.Id == botId)
                {
                    BotSet.IdleBots.Remove(bot);
                    BotSet.LockedBots.Add(bot);
                    return bot;
                }
            }
            return null;
        }
    }
}
