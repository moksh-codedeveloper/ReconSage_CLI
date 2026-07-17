namespace DBModel
{
    public class Model
    {
        public int Id{set;get;}
        public string Target{set;get;} = string.Empty;
        public string JsonFilePath{set;get;} = string.Empty;
        public string HeadersFile{set;get;} = string.Empty;
        public string WordlistsPath{set;get;} = string.Empty;
        public string HtmlFilePath{set;get;} = string.Empty;
        public string ReasonPhrase{set;get;} = string.Empty;
        public int StatusCode{set;get;}
        public double LatencyMs{set;get;}
    }

    public class CompilerDataModel
    {
        public List<string> ReasonPhrase = new();
        public string Domain = string.Empty;
        public List<double> LatencyList = new();
        public List<int> StatusCodes = new();
    }
    public class MainModel
    {
        public List<Model> packet{set;get;} = new();
    }
}