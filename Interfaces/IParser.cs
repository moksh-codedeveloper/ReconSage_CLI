namespace IParser
{
    public interface IFileParser<T>
    {
        T ParseDictToModel();
    }
}