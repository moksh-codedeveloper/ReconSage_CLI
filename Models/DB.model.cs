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
        public List<string> ReasonPhrase{set;get;} = new();
        public string Domain{set;get;} = string.Empty;
        public List<double> LatencyList{set;get;} = new();
        public List<int> StatusCodes{set;get;} = new();
        public int TotalRecords{set;get;}
    }
    public class MainModel
    {
        public List<Model> packet{set;get;} = new();
    }
}