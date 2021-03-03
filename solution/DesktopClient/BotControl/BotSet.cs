using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DesktopClient.BotControl
{
    class BotSet
    {
        public static List<BotMain> AllBots = new List<BotMain>();
        public static List<BotMain> IdleBots = new List<BotMain>();
        public static List<BotMain> SharedBots = new List<BotMain>();
        public static List<BotMain> LockedBots = new List<BotMain>();
    }
}
