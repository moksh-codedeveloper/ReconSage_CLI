namespace ResoModel
{
    public class RModel
    {
        public int Timeout{set;get;} 
        public string JsonFilePath{set;get;} = string.Empty;
        public string WordlistPath{set;get;} = string.Empty;
        public int Delay{set;get;}

        public string HeadersFile{set;get;} = string.Empty;
        public string HtmlFile{set;get;} = string.Empty;
    }
}