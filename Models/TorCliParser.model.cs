namespace TorCliModel
{
    public class TorCliParserModel
    {
        public string  Target{set;get;} = string.Empty;
        public int Timeout{set;get;}
        public double LatencyMS{set;get;}
        public string JsonFilePath{set;get;} = string.Empty;
        public string WordlistPath{set;get;} = string.Empty;
        public string host{set;get;} =  string.Empty;
        public int port{set;get;}
        public string password{set;get;} = string.Empty;
        public string tor_ip{set;get;} = string.Empty;
        public int tor_port{set;get;}
    }
}