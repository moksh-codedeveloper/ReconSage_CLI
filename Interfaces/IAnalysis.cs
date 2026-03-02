namespace Analysis
{
    public interface IAnalysis<T>
    {
        Task<T> RunAnalysis(string jsonFilePath);
    } 
}