using System.Runtime.InteropServices;
using IParser;
using ReconSageLogger;
using RfoModel;

namespace TorConfigParser
{
    // Mirror of C++ struct — field order and sizes match EXACTLY
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct CppParserConfig
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string target;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string password;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string tor_ip;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string proto_port;

        // Added missing field to match the updated C++ struct sequence
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string dns_server;

        public ushort cp_port;
        public ushort tor_port;
    }

    public class RfoParser : IFileParser<RfoParsedModel>
    {
        // P/Invoke bridge updated to handle stack allocation via "out" keyword
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern CppParserConfig parse_rfo(string filename);

        // free_parser is completely removed since memory is allocated on the C# stack frame!

        public string _filepath { set; get; } = string.Empty;

        public RfoParser(string filepath)
        {
            _filepath = filepath;
        }

        private CppParserConfig ParseViaCpp()
        {
            CppParserConfig config = parse_rfo(_filepath);
            return config;
        }

        public RfoParsedModel ParseDictToModel()
        {
            CppParserConfig parsed = ParseViaCpp();
            RfoParsedModel parsedModel = new RfoParsedModel();
            parsedModel.Target = parsed.target;
            parsedModel.Port = parsed.cp_port;
            parsedModel.Password = parsed.password;
            parsedModel.tor_ip = parsed.tor_ip;
            parsedModel.tor_port = parsed.tor_port;
            parsedModel.Proto_port = parsed.proto_port;
            parsedModel.dns_server = parsed.dns_server;

            return parsedModel;
        }
    }
}