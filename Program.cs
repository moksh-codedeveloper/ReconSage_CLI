using ReconSageShell;
public class Program
{
    public static async Task Main(string[] args)
    {
        SQLitePCL.Batteries_V2.Init();
        RecoShell app = new RecoShell();
        await app.Launch();
    }
}