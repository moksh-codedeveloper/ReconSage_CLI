using System;
using System.Threading;
using System.Threading.Tasks;
using AllFilesWires;
using ReconSageLogger;
using ReconSageShell.Commands;
using SessionData_RecoShell;
using ReconSageShell.UI;

namespace ReconSageShell
{
    public class RecoShell
    {
        private bool _isRunning = true;
        private readonly SessionData _sessionData = new();
        private readonly Wires _fileWires = new();
        private readonly CancellationTokenSource _cts = new();
        private readonly CommandRegistry _registry = new();

        public async Task Launch()
        {
            _registry.Register(
                _sessionData,
                _fileWires,
                _cts,
                onExit: () => { _isRunning = false; Logger.Done("ReconSage signing off."); },
                onBanner: () => ShellUi.RenderBanner(_sessionData),
                onHelp: () => ShellUi.RenderHelp(_registry.AllCommands)
            );

            ShellUi.RenderBanner(_sessionData);

            while (_isRunning)
            {
                ShellUi.RenderPrompt(_sessionData);

                string input = Console.ReadLine()?.Trim() ?? "";
                if (string.IsNullOrEmpty(input)) continue;

                // Instant bash/zsh command passthrough via '!'
                if (input.StartsWith("!"))
                {
                    string hostCmd = input.Substring(1).Trim();
                    if (!string.IsNullOrEmpty(hostCmd))
                    {
                        var proc = System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
                        {
                            FileName = Environment.GetEnvironmentVariable("SHELL") ?? "/bin/zsh",
                            Arguments = $"-c \"{hostCmd.Replace("\"", "\\\"")}\"",
                            UseShellExecute = false
                        });
                        proc?.WaitForExit();
                    }
                    continue;
                }

                string[] parts = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                string cmdName = parts[0].ToLower();
                string[] args = parts.Length > 1 ? parts[1..] : Array.Empty<string>();

                if (_registry.TryGetCommand(cmdName, out var command) && command != null)
                {
                    try
                    {
                        await command.Handler(args);
                    }
                    catch (Exception ex)
                    {
                        Logger.Error($"Command execution failed: {ex.Message}");
                    }
                }
                else
                {
                    Logger.Warn($"Unknown command: {cmdName}. Type 'help' for available commands.");
                }
            }
        }
    }
}