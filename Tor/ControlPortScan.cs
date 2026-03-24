using Interface.Network;
using ScanOutputModel;
namespace ControlPortUse
{
    public class ControlPortTorScan : INetwork
    {
        private readonly string Target;
        private readonly int Timeout;
        private readonly int Port;
        private readonly string Password;
        private readonly string Host;
        private readonly string TorIP;
        private readonly int TorPort;
        
        public async Task<ScanOutput> SendAsync(string domain)
        {
            return new ScanOutput();
        }
    }
}