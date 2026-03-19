namespace IParser
{
    public interface ICLIParser<T>
    {
        T ArgsProcess(string[] args);
    }
    public interface IFileParser<T>
    {
        T ParseDictToModel(Dictionary<string, string> parsedData);
    }
}