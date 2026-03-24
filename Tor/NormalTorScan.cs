using Interface.Network;
using ScanOutputModel;

namespace NormalTorScan
{
    public class TorScan : INetwork
    {
        public async Task<ScanOutput> SendAsync(string Domain)
        {
            return new ScanOutput();
        }
    }
}