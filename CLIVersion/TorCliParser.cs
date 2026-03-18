using System.Net;

namespace TcpCliWrapper
{
    public class TcpCli
    {
        private string Host = string.Empty;
        private int Port;
        private int Timeout;
        private string Target = string.Empty;
        private string WordlistPath = string.Empty;
        private string JsonFilePath = string.Empty;
        private string Password = string.Empty;
        private string tor_ip = string.Empty;
        private int tor_port;
        public void argsProcess(string[] args)
        {
            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i])
                {
                    case "--target":
                        Target = args[i + 1];
                        break;
                    case "--timeout":
                        if (!int.TryParse(args[i + 1], out int timeout))
                            throw new Exception("Actually do you know that the timeout values is suppose to be the integers if you forgot then its reminder");
                        Timeout = timeout;
                        break;
                    case "--host":
                        string host = args[i + 1];
                        if (string.IsNullOrWhiteSpace(host) || host.Length == 0 || host == "")
                            throw new Exception("Pass a legitimate host which can be used for the tor control port");
                        Host = host;
                        break;
                    case "--port":
                        if (!int.TryParse(args[i + 1], out int port))
                            throw new Exception("Actually do you know that the port are suppose to be the integers if you forgot then its reminder");
                        Port = port;
                        break;
                    case "--password":
                        string pass = args[i + 1];
                        if (string.IsNullOrWhiteSpace(pass))
                            throw new Exception("You have null as your pass i don't buy it");
                        Password = pass;
                        break;
                    case "--tor-ip":
                        string _tor_ip = args[i + 1];
                        if (!IPAddress.TryParse(_tor_ip, out IPAddress ipAddress))
                        {
                            throw new Exception("You provided an invalid IP address for tor-ip");
                        }
                        tor_ip = _tor_ip;
                        break;
                    case "--tor-port":
                        if (!int.TryParse(args[i + 1], out int _tor_port))
                        {
                            throw new Exception("Actually the tor port should be int not any other type of data type you  should study first then come here kid");
                        }
                        tor_port = _tor_port;
                        break;
                    case "--json":
                        string filepath = args[i + 1];
                        if (!filepath.EndsWith(".json", StringComparison.OrdinalIgnoreCase) || string.IsNullOrWhiteSpace(filepath))
                            throw new Exception("You serriously don't know what the json is huhh go home kid and remove this tool seriously");
                        JsonFilePath = args[i + 1];
                        break;
                    case "--wordlist":
                        string wordlistpath = args[i + 1];
                        if (!wordlistpath.EndsWith(".txt", StringComparison.OrdinalIgnoreCase) || string.IsNullOrWhiteSpace(wordlistpath))
                            throw new Exception("You serriously don't know what the wordlist is huhh go home kid and remove this tool seriously");
                        WordlistPath = wordlistpath;
                        break;
                    default:
                        Console.WriteLine("[--==]It seems you haven't read the documentation here it is just read  it first then start for the tor related operations..");
                        break;
                }
            }
        }
    }
}