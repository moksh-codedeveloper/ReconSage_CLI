using IParser;
using ResoModel;

namespace ResoParser
{
    public class RsoParser : IFileParser<RModel>
    {
        public string RsoFilePath { set; get; } = string.Empty;
        public RsoParser(string filepath)
        {
            RsoFilePath = filepath;
        }

        public Dictionary<string, string> Parse()
        {
            Dictionary<string, string> data = new();

            foreach (var line in File.ReadAllLines(RsoFilePath))
            {
                if (string.IsNullOrWhiteSpace(line) || line.StartsWith("#"))
                    continue;

                var parts = line.Split("=", 2);
                if (parts.Length == 2)
                {
                    data[parts[0].Trim()] = parts[1].Trim();
                }
            }

            // REQUIRED KEYS CHECK
            string[] requiredKeys = { "target", "concurrency", "timeout", "json_file_path", "wordlist_path", "delay"};

            foreach (var key in requiredKeys)
            {
                if (!data.ContainsKey(key))
                    throw new Exception($"Missing required key: {key}");
            }
            // TARGET validation
            if(!Uri.TryCreate(data["target"], UriKind.Absolute, out Uri? uri) || (uri.Scheme != Uri.UriSchemeHttp && uri.Scheme != Uri.UriSchemeHttps))
            {
                throw new Exception("Target has broken http url or  it is not Http Url please pass a proper url of https or http");
            }
            // NUMBER VALIDATION
            if (!int.TryParse(data["concurrency"], out int concurrency) || concurrency <= 0)
                throw new Exception("Invalid concurrency value.");
    
            if (!int.TryParse(data["timeout"], out int timeout) || timeout <= 0)
                throw new Exception("Invalid timeout value.");
            
            if(!int.TryParse(data["delay"], out int delay))
                throw new Exception("This value is not valid so pass a legit number value for delay");

            // FILE EXTENSION CHECKS
            if (!data["wordlist_path"].EndsWith(".txt", StringComparison.OrdinalIgnoreCase))
                throw new Exception("Wordlist must be .txt file.");

            if (!data["json_file_path"].EndsWith(".json", StringComparison.OrdinalIgnoreCase))
                throw new Exception("Output file must be .json.");
            return data;
        }

        public RModel ParseDictToModel()
        {
            Dictionary<string, string> data = Parse();
            return new RModel
            {
                Target = data["target"],
                Concurrency = int.Parse(data["concurrency"]),
                Timeout = int.Parse(data["timeout"]),
                JsonFilePath = data["json_file_path"],
                WordlistPath = data["wordlist_path"],
                Delay = int.Parse(data["delay"]),
            };
        }
    }
}