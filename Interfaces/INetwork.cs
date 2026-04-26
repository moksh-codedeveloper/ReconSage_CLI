using ScanOutputModel;

namespace Interface.Network
{
    public interface INetwork
    {
        Task<ScanOutput> SendAsync(string Domain, CancellationToken ct);
    }
    public interface ITlsScan
    {
        Task<TlsScanResult> TlsScan(string Domain);
    }
}