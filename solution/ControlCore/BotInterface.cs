using System;

namespace ControlCore
{
    public delegate void IsAvailableChangedEventHandler(IBot sender, bool isAvailable);

    public delegate void ModAddedEventHandler(IBot sender, IMod mod);
    public delegate void ModRemovedEventHandler(IBot sender, IMod mod);
    public delegate void ModIdChangedEventHandler(IBot sender, IMod mod, string oldId, string newId);
    public delegate void ModStateChangedEventHandler(IBot sender, IMod mod);
    public delegate void ProgramStateChangedEventHandler(IBot sender, ProgramState state);
    public delegate void ProgramOutputEventHandler(IBot sender, string output);

    public interface IBot
    {
        bool IsAvailable { get; }
        event IsAvailableChangedEventHandler IsAvailableChanged;
        string GetLastAvalabilityError();

        IMod[] GetMods();
        IMod GetMod(string id);
        event ModAddedEventHandler OnModAdded;
        event ModRemovedEventHandler OnModRemoved;
        event ModIdChangedEventHandler OnModIdChanged;

        ProgramState GetProgramState();
        string GetProgram();
        void SetProgram(string code);
        void StartProgram();
        void StopProgram();
        event ProgramStateChangedEventHandler OnProgramStateChanged;
        string GetLastCompileError();

        void SendProgramInput(string input);
        event ProgramOutputEventHandler OnProgramOutput;
    }

    public struct ProgramState
    {
        public readonly bool Running;
        public readonly bool Compiling;
        public readonly bool Compiled;
    }

    public interface IMod
    {
        string Type { get; }
        string GlobalId { get; }
        string Id { get; set; }
        event ModStateChangedEventHandler OnModStateChanged;
    }

    public interface IServo180 : IMod
    {
        double Angle { get; }
    }

    public interface ITimer : IMod
    {
        void Reset();
        long GetTime();
    }
}
