using ControlCore;
using System;

namespace ProgramLib
{
    public class Bot : IBot
    {
        public static readonly Bot Primary = new Bot();

        public bool IsAvailable => throw new NotImplementedException();

        public event ModAddedEventHandler OnModAdded;
        public event ModRemovedEventHandler OnModRemoved;
        public event ModIdChangedEventHandler OnModIdChanged;
        public event ProgramStateChangedEventHandler OnProgramStateChanged;
        public event ProgramOutputEventHandler OnProgramOutput;
        public event IsAvailableChangedEventHandler IsAvailableChanged;

        public string GetLastAvalabilityError()
        {
            throw new NotImplementedException();
        }

        public string GetLastCompileError()
        {
            throw new NotImplementedException();
        }

        public IMod GetMod(string id)
        {
            throw new NotImplementedException();
        }

        public IMod[] GetMods()
        {
            throw new NotImplementedException();
        }

        public string GetProgram()
        {
            throw new NotImplementedException();
        }

        public ProgramState GetProgramState()
        {
            throw new NotImplementedException();
        }

        public void SendProgramInput(string input)
        {
            throw new NotImplementedException();
        }

        public void SetProgram(string code)
        {
            throw new NotImplementedException();
        }

        public void StartProgram()
        {
            throw new NotImplementedException();
        }

        public void StopProgram()
        {
            throw new NotImplementedException();
        }
    }
}
