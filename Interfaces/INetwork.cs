using ScanOutputModel;

namespace Interface.Network
{
    public interface INetwork
    {
        Task<ScanOutput> SendAsync(string Domain);
    }
    public interface ITlsScan
    {
        Task<TlsScanResult> TlsScan(string Domain);
    }
}