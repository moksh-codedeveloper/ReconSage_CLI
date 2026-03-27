namespace RfoModel
{
    public class RfoParsedModel
    {
        public string Target{set;get;} = string.Empty;
        public string JsonFilePath{set;get;} = string.Empty;
        public int Port{set;get;} 
        public int Timeout{set;get;}
        public string Password{set;get;} = string.Empty; 
        public string WordlistPath{set;get;} = string.Empty;
        public string host{set;get;} = string.Empty;
        public string tor_ip{set;get;} = string.Empty;
        public int tor_port{set;get;}
        public int delay{set;get;}
    }
}