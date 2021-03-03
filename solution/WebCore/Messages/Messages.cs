using ControlCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WebCore.Messages
{
    public enum BotAssignment
    {
        Idle,
        Locked,
        Shared
    }

    public class CommMessage
    {
        public virtual void Handle(CommContext context)
        {
        }
    }

    public class SetSession : CommMessage
    {
        public string SessionId;
    }

    public class LockBot : CommMessage
    {
        public string BotId;

        public override void Handle(CommContext context)
        {
            IBot bot = context.LockBot(BotId);
            if(bot != null)
            {
                context.Send(new BotLocked { BotId = BotId });
            }
            else
            {
                context.Send(new BotLockFailed { BotId = BotId });
            }
        }
    }

    public class BotLocked : CommMessage
    {
        public string BotId;
    }

    public class BotLockFailed : CommMessage
    {
        public string BotId;
    }

    public class BotState : CommMessage
    {
        public string BotId;
        public BotAssignment Assignment;
        public bool IsAvailable;
        public string LastAvalabilityError;
        public ProgramState ProgramState;
    }

    public class ModId : CommMessage
    {
        public bool Exists;
        public string Type;
        public string GlobalId;
        public string Id;
    }

    public class ModState : CommMessage
    {
        public string GlobalId;
    }

    public class TimerState : ModState
    {
        public long Time;
    }

    public class ModCommand : CommMessage
    {
        public string GlobalId;
    }

    public class ResetTimer : ModCommand
    {
    }
}
