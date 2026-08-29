using System;
using System.Threading.Tasks;

namespace Commands.RecoShell
{
    public class ShellCommand
    {
        public string Name { get; set; } = "";
        public string Category { get; set; } = "";
        public string Description { get; set; } = "";
        public string Usage { get; set; } = "";
        public Func<string[], Task> Handler { get; set; } = _ => Task.CompletedTask;
    }
}