namespace ResoModel
{
    public class RModel
    {
        public string Target{set;get;} = string.Empty;
        public int Concurrency{set;get;}
        public int Timeout{set;get;} 
        public string JsonFilePath{set;get;} = string.Empty;
        public string WordlistPath{set;get;} = string.Empty;
        public int Delay{set;get;}
    }
}