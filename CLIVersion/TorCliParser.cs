using System.Net;
using IParser;
using TorCliModel;

namespace TcpCliWrapper
{
    public class TcpCli : ICLIParser<TorCliParserModel>
    {
        public TorCliParserModel ArgsProcess(string[] args)
        {
            var torParserModel = new TorCliParserModel();
            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i])
                {
                    case "--target":
                        torParserModel.Target = args[i + 1];
                        break;
                    case "--timeout":
                        if (!int.TryParse(args[i + 1], out int timeout))
                            throw new Exception("Actually do you know that the timeout values is suppose to be the integers if you forgot then its reminder");
                        torParserModel.Timeout = timeout;
                        break;
                    case "--host":
                        string host = args[i + 1];
                        if (string.IsNullOrWhiteSpace(host) || host.Length == 0 || host == "")
                            throw new Exception("Pass a legitimate host which can be used for the tor control port");
                        torParserModel.host = host;
                        break;
                    case "--port":
                        if (!int.TryParse(args[i + 1], out int port))
                            throw new Exception("Actually do you know that the port are suppose to be the integers if you forgot then its reminder");
                        torParserModel.port = port;
                        break;
                    case "--password":
                        string pass = args[i + 1];
                        if (string.IsNullOrWhiteSpace(pass))
                            throw new Exception("You have null as your pass i don't buy it");
                        torParserModel.password = pass;
                        break;
                    case "--tor-ip":
                        string _tor_ip = args[i + 1];
                        if (!IPAddress.TryParse(_tor_ip, out IPAddress ipAddress))
                        {
                            throw new Exception("You provided an invalid IP address for tor-ip");
                        }
                        torParserModel.tor_ip = _tor_ip;
                        break;
                    case "--tor-port":
                        if (!int.TryParse(args[i + 1], out int _tor_port))
                        {
                            throw new Exception("Actually the tor port should be int not any other type of data type you  should study first then come here kid");
                        }
                        torParserModel.tor_port = _tor_port;
                        break;
                    case "--json":
                        string filepath = args[i + 1];
                        if (!filepath.EndsWith(".json", StringComparison.OrdinalIgnoreCase) || string.IsNullOrWhiteSpace(filepath))
                            throw new Exception("You serriously don't know what the json is huhh go home kid and remove this tool seriously");
                        torParserModel.JsonFilePath = args[i + 1];
                        break;
                    case "--wordlist":
                        string wordlistpath = args[i + 1];
                        if (!wordlistpath.EndsWith(".txt", StringComparison.OrdinalIgnoreCase) || string.IsNullOrWhiteSpace(wordlistpath))
                            throw new Exception("You serriously don't know what the wordlist is huhh go home kid and remove this tool seriously");
                        torParserModel.WordlistPath = wordlistpath;
                        break;
                    default:
                        Console.WriteLine("[--==]It seems you haven't read the documentation here it is just read  it first then start for the tor related operations..");
                        break;
                }
            }
            return torParserModel;
        }
    }
}