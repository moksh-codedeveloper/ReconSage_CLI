using System;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.IO;
using System.Threading.Tasks;
using AllFilesWires;
using AllScansInOne;
using CompilerToDB;
using Reco_GAN_Native;
using ReconSageLogger;
using SessionData_RecoShell;
using Commands.RecoShell;
using Reco_GAN_Waf_Catcher;
namespace ReconSageShell.Commands
{
    public class CommandRegistry
    {
        private readonly Dictionary<string, ShellCommand> _commands = new(StringComparer.OrdinalIgnoreCase);

        public IEnumerable<ShellCommand> AllCommands => _commands.Values;

        public void Register(SessionData sessionData, Wires fileWires, CancellationTokenSource cts, Action onExit, Action onBanner, Action onHelp)
        {
            // Core
            Add("help", "Core", "Display available commands", _ => { onHelp(); return Task.CompletedTask; });
            Add("banner", "Core", "Display the framework banner", _ => { onBanner(); return Task.CompletedTask; });
            Add("exit", "Core", "Exit the framework", _ => { onExit(); return Task.CompletedTask; });

            // Loaders
            Add("load_rso", "Loaders", "Load RSO config file", args =>
            {
                if (args.Length < 1) { Logger.Error("Usage: load_rso <path>"); return Task.CompletedTask; }
                sessionData.LoadRso(args[0]);
                Logger.Success($"RSO Config Loaded from {args[0]}");
                return Task.CompletedTask;
            });

            Add("load_rfo", "Loaders", "Load RFO config file", args =>
            {
                if (args.Length < 1) { Logger.Error("Usage: load_rfo <path>"); return Task.CompletedTask; }
                sessionData.LoadRfo(args[0]);
                Logger.Success($"RFO Config Loaded from {args[0]}");
                return Task.CompletedTask;
            });

            Add("load_rxo", "Loaders", "Load RXO config file", args =>
            {
                if (args.Length < 1) { Logger.Error("Usage: load_rxo <path>"); return Task.CompletedTask; }
                sessionData.LoadRxo(args[0]);
                Logger.Success($"RXO Config Loaded from {args[0]}");
                return Task.CompletedTask;
            });

            // Scanners
            Add("start_scan_cpp", "Scanners", "Run native C++ scan module", async _ =>
            {
                if (!sessionData.isRsoLoaded || !sessionData.isRfoLoaded) { Logger.Warn("RSO and RFO data not loaded!"); return; }
                Logger.Scan("Initializing C++ Scan Module...");
                var cppRso = sessionData.rsoModel!;
                var cppRfo = sessionData.rfoModel!;
                string cpp_headers = await fileWires.HeaderTextParser(cppRso.HeadersFile);
                await AllScans.ExecCppScan(cppRfo.Target, cppRfo.Proto_port, cpp_headers, cppRso.Timeout, cppRso.Delay, cppRso.JsonFilePath, cppRso.WordlistPath, cts, cppRfo.dns_server);
            });

            Add("start_tor_scan", "Scanners", "Run Tor-routed scan module", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); return; }
                Logger.Scan("Initializing Tor Scan Module...");
                var torRso = sessionData.rsoModel!;
                var torRfo = sessionData.rfoModel!;
                await AllScans.ExecTorScan(torRso.JsonFilePath, torRso.WordlistPath, torRfo.Target, torRfo.Password, torRfo.tor_ip, torRfo.Proto_port, torRfo.tor_port, torRfo.Port, torRso.Timeout, torRso.Delay, cts);
            });

            Add("start_http_proxy_scan", "Scanners", "Run HTTP proxy scan module", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); return; }
                Logger.Scan("Initializing Http Proxy Scan Module...");
                var httpRso = sessionData.rsoModel!;
                var httpRfo = sessionData.rfoModel!;
                string headers = await fileWires.HeaderTextParser(httpRso.HeadersFile);
                await AllScans.ExecHttpProxy(httpRfo.Target, httpRfo.Proto_port, httpRfo.tor_ip, headers, httpRso.JsonFilePath, httpRso.WordlistPath, httpRso.Timeout, httpRso.Delay, httpRfo.tor_port, cts);
            });

            Add("start_socks_proxy_scan", "Scanners", "Run SOCKS proxy scan module", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); return; }
                Logger.Scan("Initializing Socks Proxy Scan Module....");
                var socksRso = sessionData.rsoModel!;
                var socksRfo = sessionData.rfoModel!;
                string socks_headers = await fileWires.HeaderTextParser(socksRso.HeadersFile);
                await AllScans.ExecSockProxy(socksRfo.Target, socksRfo.Proto_port, socksRfo.tor_ip, socks_headers, socksRso.JsonFilePath, socksRso.WordlistPath, socksRso.Timeout, socksRfo.tor_port, cts, socksRso.Delay);
            });

            Add("start_cpp_body_capture", "Scanners", "Capture body contents using C++ engine", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); return; }
                Logger.Scan("Initializing Cpp body capture scan module......");
                var bodySocksRso = sessionData.rsoModel!;
                var bodySocksRfo = sessionData.rfoModel!;
                await AllScans.ExecCaptureScan(bodySocksRfo.Target, bodySocksRfo.Proto_port, bodySocksRfo.tor_ip, bodySocksRso.HtmlFile, bodySocksRso.WordlistPath, bodySocksRso.Timeout, bodySocksRfo.tor_port, cts, bodySocksRfo.dns_server);
            });

            // Database Operations
            Add("transfer_json_to_db", "Database", "Transfer JSON scan results to database", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO and RSO data not loaded!"); return; }
                Logger.Info("Initializing the DB module and start up will begin soon.....");
                var db_rso = sessionData.rsoModel!;
                var db_rfo = sessionData.rfoModel!;
                var db_rxo = sessionData.rxoModel!;
                await JsonToDB.JsonFileToDB(db_rso.JsonFilePath, db_rso.WordlistPath, db_rso.HtmlFile, db_rso.HeadersFile, db_rfo.Target, db_rxo.db_password);
            });

            Add("compile_db_and_based_on_status_code_save", "Database", "Compile DB filtered by status code", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); return; }
                Logger.Info("Initializing the Compilation of DB Module.....");
                await JsonToDB.StatusCodeBasedFilterCompile(sessionData.rfoModel!.Target, sessionData.rxoModel!.db_password, sessionData.rxoModel!.status_code);
            });

            Add("compile_db_and_based_on_latency_save", "Database", "Compile DB filtered by latency", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); return; }
                Logger.Info("Initializing the Compilation of DB Module.....");
                await JsonToDB.StatusCodeBasedFilterCompile(sessionData.rfoModel!.Target, sessionData.rxoModel!.db_password, sessionData.rxoModel!.status_code);
            });

            // Machine Learning / GAN
            Add("reco_gan_training", "GAN Engine", "Train GAN model on reconnaissance data", _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); return Task.CompletedTask; }
                new Reco_GAN(sessionData.rfoModel!.Target, sessionData.rxoModel!.k_factor).Training();
                return Task.CompletedTask;
            });

            Add("reco_gan_predict", "GAN Engine", "Run predictions using trained GAN model", _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); return Task.CompletedTask; }
                new Reco_GAN(sessionData.rfoModel!.Target, sessionData.rxoModel!.k_factor).Predict();
                return Task.CompletedTask;
            });

            Add("reco_gan_trees_train", "GAN Engine", "Train decision forest on latency data", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); return; }
                Logger.Info("Training starts and creation of trees starts in a text file");
                await JsonToDB.LatTrain(sessionData.rfoModel!.Target, sessionData.rxoModel!.num_trees, sessionData.rxoModel!.sub_sample_size, sessionData.rxoModel!.db_password);
            });

            Add("reco_gan_trees_predict", "GAN Engine", "Predict latency anomalies via decision forest", async _ =>
            {
                if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data_files not loaded!"); return; }
                Logger.Info("Prediction on the following latency Data starts this minute...");
                await JsonToDB.LatPredict(sessionData.rfoModel!.Target, sessionData.rxoModel!.sub_sample_size, sessionData.rxoModel!.db_password);
            });
            Add("reco_gan_catch_waf", "GAN Engine", "Deterministic Waf Catcher using Response Body we gathered from the targetted domain names", async _ =>
            {
                if (!sessionData.isRsoLoaded || !sessionData.isRxoLoaded || !sessionData.isRfoLoaded) { Logger.Warn("RFO RXO RSO data not loaded!"); }
                Logger.Info("Waf Catching starts right here and now ....");
                await JsonToDB.CatchWaf(sessionData.rfoModel!.Target, sessionData.rxoModel!.sub_sample_size, sessionData.rxoModel!.db_password, sessionData.rsoModel!.HtmlFile);
            });
            Add("shell", "OS Integration", "Spawn an interactive subshell session (zsh/bash)", _ => SpawnInteractiveSubshell());

            Add("exec", "OS Integration", "Execute a host command (e.g., exec ls -la)", async args =>
            {
                if (args.Length == 0)
                {
                    Logger.Error("Usage: exec <command>");
                    return;
                }
                string cmd = string.Join(" ", args);
                await RunHostCommand(cmd);
            });
        }

        private void Add(string name, string category, string description, Func<string[], Task> handler)
        {
            _commands[name] = new ShellCommand
            {
                Name = name,
                Category = category,
                Description = description,
                Handler = handler
            };
        }

        public bool TryGetCommand(string name, out ShellCommand? command) => _commands.TryGetValue(name, out command);
        private static string GetUserShell()
        {
            // Detect user's active shell from environment or fallback
            string? shell = Environment.GetEnvironmentVariable("SHELL");
            if (!string.IsNullOrEmpty(shell) && File.Exists(shell))
                return shell;

            return File.Exists("/bin/zsh") ? "/bin/zsh" : "/bin/bash";
        }

        private static async Task RunHostCommand(string fullCommand)
        {
            try
            {
                string shell = GetUserShell();
                var processInfo = new ProcessStartInfo
                {
                    FileName = shell,
                    Arguments = $"-c \"{fullCommand.Replace("\"", "\\\"")}\"",
                    UseShellExecute = false,
                    RedirectStandardOutput = false,
                    RedirectStandardError = false,
                    RedirectStandardInput = false
                };

                using var proc = Process.Start(processInfo);
                if (proc != null)
                {
                    await proc.WaitForExitAsync();
                }
            }
            catch (Exception ex)
            {
                Logger.Error($"Shell execution failed: {ex.Message}");
            }
        }

        private static async Task SpawnInteractiveSubshell()
        {
            try
            {
                string shell = GetUserShell();
                Logger.Info($"Spawning interactive shell ({shell}). Type 'exit' to return to ReconSage.");

                var processInfo = new ProcessStartInfo
                {
                    FileName = shell,
                    UseShellExecute = false
                };

                using var proc = Process.Start(processInfo);
                if (proc != null)
                {
                    await proc.WaitForExitAsync();
                }

                Logger.Success("Returned to ReconSage shell.");
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to spawn subshell: {ex.Message}");
            }
        }
    }
}