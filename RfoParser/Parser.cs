using System.Net;
using System.Net.Sockets;
using IParser;
using RfoModel;

namespace TorConfigParser
{
    public class RfoParser : IFileParser<RfoParsedModel>
    {
        public string _filepath { set; get; } = string.Empty;
        public RfoParser(string filepath)
        {
            _filepath = filepath;
        }
        private Dictionary<string, string> Parse()
        {
            Dictionary<string, string> data = new Dictionary<string, string>();
            if (!_filepath.EndsWith(".rfo"))
            {
                throw new Exception("The file you have passed doesn't seem to have name config.rfo so please name it and pass it proper file");
            }

            var line = File.ReadAllLines(_filepath);
            foreach (var words in line)
            {
                if (string.IsNullOrWhiteSpace(words) || words.StartsWith("#"))
                    continue;
                var parts = words.Split("=", 2);
                if (parts.Length == 2)
                {
                    data[parts[0].Trim()] = parts[1].Trim();
                }
            }
            List<string> requiredData = new() { "host", "port", "password", "target", "timeout", "json_file_path", "wordlist_path", "tor_ip", "tor_port", "delay" };
            foreach (var keys in requiredData)
                if (!data.ContainsKey(keys))
                    throw new Exception("you it seems like you have passed the wrong file which was not supposed to be passed here please pass config.rfo");

            if (!int.TryParse(data["port"], out int port))
                throw new Exception("Invalid concurrency value.");

            if (!int.TryParse(data["timeout"], out int timeout) || timeout <= 0)
                throw new Exception("The provided timeout value is not acceptable here so yeah please pass something legit");

            if (data["password"] == "" || string.IsNullOrWhiteSpace(data["password"]) || data["password"].Length == 0)
                throw new Exception("Please pass a valid password");

            if (!Uri.TryCreate(data["target"], UriKind.Absolute, out Uri? target) || (target.Scheme != Uri.UriSchemeHttp && target.Scheme != Uri.UriSchemeHttps))
                throw new Exception("Target has broken http url or  it is not Http Url please pass a proper url of https or http");

            IPAddress? ip = null;
            bool valid = IPAddress.TryParse(data["host"], out ip);
            if (!valid)
                throw new Exception("There is something wrong with the ip you passed here in host so yeah please verify");

            if (!int.TryParse(data["tor_port"], out int _tor_port))
                throw new Exception("You passed  tor port value not in number please pass it  in number not in anyother data types");
            IPAddress? tor_ip = null;
            bool validTorIP = IPAddress.TryParse(data["tor_ip"], out tor_ip);
            if (!validTorIP)
                throw new Exception("There is something wrong with the ip you passed here in top_ip so yeah please verify");

            // FILE EXTENSION CHECKS
            if (!data["wordlist_path"].EndsWith(".txt", StringComparison.OrdinalIgnoreCase))
                throw new Exception("Wordlist must be .txt file.");

            if (!data["json_file_path"].EndsWith(".json", StringComparison.OrdinalIgnoreCase))
                throw new Exception("Output file must be .json.");
            if (!int.TryParse(data["delay"], out int delay)) throw new Exception("Value error: delay is not integer but it should be integer not any other value");

            return data;
        }
        public RfoParsedModel ParseDictToModel()
        {
            Dictionary<string, string> parsedData = Parse();
            RfoParsedModel parsedModel = new RfoParsedModel();
            parsedModel.Target = parsedData["target"];
            parsedModel.Timeout = int.Parse(parsedData["timeout"]);
            parsedModel.JsonFilePath = parsedData["json_file_path"];
            parsedModel.WordlistPath = parsedData["wordlist_path"];
            parsedModel.Port = int.Parse(parsedData["port"]);
            parsedModel.host = parsedData["host"];
            parsedModel.Password = parsedData["password"];
            parsedModel.tor_ip = parsedData["tor_ip"];
            parsedModel.tor_port = int.Parse(parsedData["tor_port"]);
            parsedModel.delay = int.Parse(parsedData["delay"]);
            return parsedModel;
        }
    }
}