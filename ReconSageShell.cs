using AllScansInOne;
using IParser;
using ReconSageLogger;
using ResoModel;
using ResoParser;
using RfoModel;
using TorConfigParser;
using CompilerToDB;
using Reco_GAN_Native;
using AllFilesWires;
using RxoModel;
using RxoParser;
namespace ReconSageShell
{
    public class SessionData
    {
        public RModel? RsoConfig { set; get; }
        public RfoParsedModel? rfoParsed { set; get; }
        public Model? RxoParse { set; get; }
        public bool isRsoLoaded => RsoConfig != null;
        public bool isRfoLoaded => rfoParsed != null;
        public bool isRxoLoaded => RxoParse != null;

        public void LoadRxo(string filePath)
        {
            IFileParser<Model> RxoFileParser = new Parser(filePath);
            RxoParse = RxoFileParser.ParseDictToModel();
        }
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
        private void PrintBanner()
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

            string[] commands = { "load_rso", "load_rxo", "load_rfo", "start_scan_cpp", "start_tor_scan", "start_http_proxy_scan", "start_socks_proxy_scan", "start_cpp_body_capture", "transfer_json_to_db", "compile_db_and_save", "reco_gan_training", "reco_gan_predict" };
            foreach (var cmd in commands)
            {
                Console.WriteLine($"   > {cmd,-20}");
            }

            Console.ResetColor();
            Console.WriteLine("\n" + new string('-', 50) + "\n");
        }

        public async Task Launch()
        {
            PrintBanner();
            var sessionData = new SessionData();
            var cts = new CancellationTokenSource();
            var fileWires = new Wires();
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
                        case "load_rxo":
                            if (parts.Length < 2) { Logger.Error("Usage: load_rxo <path>"); break; }
                            sessionData.LoadRxo(parts[1]);
                            Logger.Success($"RXO Config Loaded from {parts[1]}");
                            break;
                        case "start_scan_cpp":
                            if (!sessionData.isRsoLoaded || !sessionData.isRfoLoaded) { Logger.Warn("RSO and RFO data not loaded!"); break; }
                            Logger.Scan("Initializing C++ Scan Module...");
                            var cppRso = sessionData.RsoConfig!;
                            var cppRfo = sessionData.rfoParsed!;
                            string cpp_headers = await fileWires.HeaderTextParser(cppRso.HeadersFile);
                            await AllScans.ExecCppScan(target: cppRfo.Target, port: cppRfo.Proto_port, headers: cpp_headers,
                            timeout: cppRso.Timeout, delay: cppRso.Delay,
                            jsonFilePath: cppRso.JsonFilePath,
                            wordlistPath: cppRso.WordlistPath, cts, cppRfo.dns_server);
                            break;

                        case "start_tor_scan":
                            if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); break; }
                            Logger.Scan("Initializing Tor Scan Module...");
                            var torRso = sessionData.RsoConfig!;
                            var torRfo = sessionData.rfoParsed!;
                            await AllScans.ExecTorScan(jsonFilePath: torRso.JsonFilePath, wordlistPath: torRso.WordlistPath, target: torRfo.Target, Password: torRfo.Password, TorIp: torRfo.tor_ip, Port: torRfo.Proto_port, TorPort: torRfo.tor_port, CpTorPort: torRfo.Port, Timeout: torRso.Timeout, Delay: torRso.Delay, cts);
                            break;
                        case "start_http_proxy_scan":
                            if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); break; }
                            Logger.Scan("Initializing Http Proxy Scan Module...");
                            var httpRso = sessionData.RsoConfig!;
                            var httpRfo = sessionData.rfoParsed!;
                            string headers = await fileWires.HeaderTextParser(httpRso.HeadersFile);
                            await AllScans.ExecHttpProxy(httpRfo.Target, httpRfo.Proto_port, httpRfo.tor_ip, headers, httpRso.JsonFilePath, httpRso.WordlistPath, httpRso.Timeout, httpRso.Delay, httpRfo.tor_port, cts);
                            break;
                        case "start_socks_proxy_scan":
                            if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); break; }
                            Logger.Scan("Initializing Socks Proxy Scan Module....");
                            var socksRso = sessionData.RsoConfig!;
                            var socksRfo = sessionData.rfoParsed!;
                            string socks_headers = await fileWires.HeaderTextParser(socksRso.HeadersFile);
                            await AllScans.ExecSockProxy(socksRfo.Target, socksRfo.Proto_port, socksRfo.tor_ip, socks_headers, socksRso.JsonFilePath, socksRso.WordlistPath, socksRso.Timeout, socksRfo.tor_port, cts, socksRso.Delay);
                            break;
                        case "start_cpp_body_capture":
                            if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded) { Logger.Warn("RFO and RSO data not loaded!"); break; }
                            Logger.Scan("Initializing Cpp body capture scan module......");
                            var bodySocksRso = sessionData.RsoConfig!;
                            var bodySocksRfo = sessionData.rfoParsed!;
                            await AllScans.ExecCaptureScan(bodySocksRfo.Target, bodySocksRfo.Proto_port, bodySocksRfo.tor_ip, bodySocksRso.HtmlFile, bodySocksRso.WordlistPath, bodySocksRso.Timeout, bodySocksRfo.tor_port, cts, bodySocksRfo.dns_server);
                            break;
                        case "transfer_json_to_db":
                            if (!sessionData.isRfoLoaded || !sessionData.isRsoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO and RSO data not loaded!"); break; }
                            Logger.Info("Initializing the DB module and start up will begin soon.....");
                            var db_rso = sessionData.RsoConfig!;
                            var db_rfo = sessionData.rfoParsed!;
                            var db_rxo = sessionData.RxoParse!;
                            await JsonToDB.JsonFileToDB(db_rso.JsonFilePath, db_rso.WordlistPath, db_rso.HtmlFile, db_rso.HeadersFile, db_rfo.Target, db_rxo.db_password);
                            break;
                        case "compile_db_and_based_on_status_code_save":
                            if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); break; }
                            Logger.Info("Initializing the Compilation of DB Module.....");
                            var reco_novich_rfo = sessionData.rfoParsed!;
                            var reco_novich_rxo = sessionData.RxoParse!;
                            await JsonToDB.StatusCodeBasedFilterCompile(reco_novich_rfo.Target, reco_novich_rxo.db_password, reco_novich_rxo.status_code);
                            break;
                        case "compile_db_and_based_on_latency_save":
                            if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); break; }
                            Logger.Info("Initializing the Compilation of DB Module.....");
                            var latency_reco_novich_rfo = sessionData.rfoParsed!;
                            var latency_reco_novich_rxo = sessionData.RxoParse!;
                            await JsonToDB.StatusCodeBasedFilterCompile(latency_reco_novich_rfo.Target, latency_reco_novich_rxo.db_password, latency_reco_novich_rxo.status_code);
                            break;
                        case "reco_gan_training":
                            if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); break; }
                            var reco_gan_rfo = sessionData.rfoParsed!;
                            var reco_gan_rxo = sessionData.RxoParse!;
                            Reco_GAN reco_GAN = new Reco_GAN(reco_gan_rfo.Target, reco_gan_rxo.k_factor);
                            reco_GAN.Training();
                            break;
                        case "reco_gan_predict":
                            if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded) { Logger.Warn("RFO RXO data not loaded!"); break; }
                            var reco_gan_predict_rfo = sessionData.rfoParsed!;
                            var reco_gan_predict_rxo = sessionData.RxoParse!;
                            Reco_GAN reco_GAN1 = new Reco_GAN(reco_gan_predict_rfo.Target, reco_gan_predict_rxo.k_factor);
                            reco_GAN1.Predict();
                            break;
                        case "reco_gan_trees_train":
                            if (!sessionData.isRfoLoaded || !sessionData.isRxoLoaded)
                            {
                                Logger.Warn("RFO RXO data not loaded!");
                                break;
                            }
                            Logger.Info("Training starts and creation of trees starts in a text file");
                            var recoGanTreeRfo = sessionData.rfoParsed!;
                            var recoGanTreeRxo = sessionData.RxoParse!;
                            await JsonToDB.LatTrain(recoGanTreeRfo.Target, recoGanTreeRxo.num_trees, recoGanTreeRxo.sub_sample_size, recoGanTreeRxo.db_password);
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