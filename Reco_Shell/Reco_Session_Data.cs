using ResoModel;
using RfoModel;
using RxoModel;
using IParser;
using RxoParser;
using ResoParser;
using TorConfigParser;

namespace SessionData_RecoShell
{
    public class SessionData
    {
        public Model? rxoModel { set; get; }
        public void LoadRxo(string filePath)
        {
            IFileParser<Model> fileParser = new Parser(filePath);
            rxoModel = fileParser.ParseDictToModel();
        }
        public RModel? rsoModel { set; get; }
        public void LoadRso(string filePath)
        {
            IFileParser<RModel> fileParser = new RsoParser(filePath);
            rsoModel = fileParser.ParseDictToModel();
        }
        public RfoParsedModel? rfoModel { set; get; }
        public void LoadRfo(string filePath)
        {
            IFileParser<RfoParsedModel> fileParser = new RfoParser(filePath);
            rfoModel = fileParser.ParseDictToModel();
        }

        public bool isRfoLoaded => rfoModel != null;
        public bool isRxoLoaded => rxoModel != null;
        public bool isRsoLoaded => rsoModel != null;
    }
}