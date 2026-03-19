namespace IParser
{
    public interface ICLIParser<T>
    {
        T ArgsProcess(string[] args);
    }
    public interface IParser<T>
    {
        T ParseDictToModel(Dictionary<string, string> parsedData);
    }
}