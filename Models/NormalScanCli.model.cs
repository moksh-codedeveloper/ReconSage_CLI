namespace NormalScanCliModel
{
    public class NormalScanCliParserModel
    {
        public string Target{set;get;} = string.Empty;
        public  int Timeout{set;get;}
        public int Concurrency{set;get;}
        public string WordlistPath{set;get;} = string.Empty;
        public string JsonFilePath{set;get;} = string.Empty;
        public int delay{set;get;}
    }
}