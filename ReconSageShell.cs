using IParser;
using ReconSageLogger;
using ResoModel;
using ResoParser;
using RfoModel;
using TorConfigParser;

namespace ReconSageShell
{
    public class SessionData
    {
        public RModel? RsoConfig { set; get; }
        public RfoParsedModel? rfoParsed { set; get; }
        public bool isRsoLoaded => RsoConfig != null;
        public bool isRfoLoaded => rfoParsed != null;
        public void LoadRso(string filePath)
        {
            IFileParser<RModel> RsoFileParser = new RsoParser(filePath);
            RsoConfig = RsoFileParser.ParseDictToModel();
        }
        public void LoadRfo(string filePath)
        {
            IFileParser<RfoParsedModel> rfoFileParser = new RfoParser(filePath);
            rfoParsed = rfoFileParser.ParseDictToModel();
        }
    }
    public class RecoShell
    {
        private bool _isRunning = true;
        public void PrintBanner()
        {
            Console.Clear();
            Console.ForegroundColor = ConsoleColor.Cyan;

            // ASCII Art Header
            Console.WriteLine(@"
    __________  ________________  _____  ___ __________ 
    ___  __ \_  / __  __ \_  __ \__  / / /_  ____/_  _ \
    __  /_/ /  /  _  / / /  / / /_  /_/ /_  / __ _  / / /
    _  _, _// /___/ /_/ // /_/ /_  __  / / /_/ / / /_/ / 
    /_/ |_|/_____/\____/ /____/ /_/ /_/  \____/  \____/  
                                         v2.0 [ARCH-LINUX]
    ");

            Console.ResetColor();
            Console.WriteLine(new string('-', 50));
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($" {'['} ReconSage | Expertise: Advanced Reconnaissance {']'} ");
            Console.ResetColor();
            Console.WriteLine(new string('-', 50));

            Console.WriteLine("\n Available Commands:");
            Console.ForegroundColor = ConsoleColor.Green;

            string[] commands = { "load_rso", "load_rfo", "start_scan_cpp", "start_tor_scan" };
            foreach (var cmd in commands)
            {
                Console.WriteLine($"   > {cmd,-20}");
            }

            Console.ResetColor();
            Console.WriteLine("\n" + new string('-', 50) + "\n");
        }

        public void Launch()
        {
            PrintBanner();
            var sessionData = new SessionData();

            while (_isRunning)
            {
                // Custom prompt with Logger
                Console.ForegroundColor = ConsoleColor.White;
                Console.Write("reconsage.1> ");
                Console.ResetColor();

                string input = Console.ReadLine()?.Trim() ?? "";
                if (string.IsNullOrEmpty(input)) continue;

                string[] parts = input.Split(' ');
                string cmd = parts[0].ToLower();

                try
                {
                    switch (cmd)
                    {
                        case "load_rso":
                            if (parts.Length < 2) { Logger.Error("Usage: load_rso <path>"); break; }
                            sessionData.LoadRso(parts[1]);
                            Logger.Success($"RSO Config Loaded from {parts[1]}");
                            break;

                        case "load_rfo":
                            if (parts.Length < 2) { Logger.Error("Usage: load_rfo <path>"); break; }
                            sessionData.LoadRfo(parts[1]);
                            Logger.Success($"RFO Config Loaded from {parts[1]}");
                            break;

                        case "start_scan_cpp":
                            if (!sessionData.isRsoLoaded) { Logger.Warn("RSO data not loaded!"); break; }
                            Logger.Scan("Initializing C++ Scan Module...");
                            // Yahan apna C++ logic call karo
                            break;

                        case "start_tor_scan":
                            if (!sessionData.isRfoLoaded) { Logger.Warn("RFO data not loaded!"); break; }
                            Logger.Scan("Initializing Tor Scan Module...");
                            break;

                        case "exit":
                            _isRunning = false;
                            Logger.Done("ReconSage signing off.");
                            break;

                        default:
                            Logger.Warn($"Unknown command: {cmd}");
                            break;
                    }
                }
                catch (Exception ex)
                {
                    Logger.Error($"Command execution failed: {ex.Message}");
                }
            }
        }
    }
}