using Core;
using DesktopClient.BotControl;
using EmbedIO;
using EmbedIO.Routing;
using EmbedIO.WebApi;
using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WebCore.Messages;

namespace DesktopClient.WebServer
{
    class LocalController : WebApiController
    {
        private readonly IDispatcher mDipatcher;
        public LocalController(IDispatcher dispatcher)
        {
            mDipatcher = dispatcher;
        }

        [Route(HttpVerbs.Get, "/ports")]
        public Task<string[]> GetPorts()
        {
            return mDipatcher.DoFunc(() =>
                {
                    List<string> unusedPorts = new List<string>();
                    string[] ports = SerialPort.GetPortNames();
                    for (int i = 0; i < ports.Length; i++)
                    {
                        bool unused = true;
                        foreach (BotMain bot in BotSet.AllBots)
                        {
                            IBotConnection con = bot.GetConnection();
                            if ((con is ComConnection) && (((ComConnection)con).GetPortName() == ports[i]))
                            {
                                unused = false;
                                break;
                            }
                        }
                        if (unused)
                        {
                            unusedPorts.Add(ports[i]);
                        }
                    }
                    return unusedPorts.ToArray();
                });
        }

        [Route(HttpVerbs.Get, "/ports")]
        public Task<BotState[]> GetBots()
        {
            return mDipatcher.DoFunc(() =>
            {
                List<BotState> bots = new List<BotState>();
                foreach (BotMain bot in BotSet.AllBots)
                {
                    BotState state = new BotState();
                    bots.Add(state);
                    state.BotId = bot.Id;
                    if (BotSet.LockedBots.Contains(bot)) state.Assignment = BotAssignment.Locked;
                    else if (BotSet.SharedBots.Contains(bot)) state.Assignment = BotAssignment.Shared;
                    else state.Assignment = BotAssignment.Idle;
                }
                return bots.ToArray();
            });
        }
    }
}
