using ReconSageShell;
public class Program
{
    public static async Task Main(string[] args)
    {
        RecoShell app = new RecoShell();
        await app.Launch();
    }
}