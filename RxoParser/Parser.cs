using System.Runtime.InteropServices;
using IParser;
using RxoModel;

namespace RxoParser
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct RxoStruct
    {
        public int status_code;
        public double k_factor;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 3096)]
        public string db_password;
        public double latency_ms;
    }
    public class Parser : IFileParser<Model>
    {
        [DllImport("parser_cpp_module.so", CallingConvention = CallingConvention.Cdecl)]
        private static extern RxoStruct rxo_parse(string fileName);
        private string file = string.Empty;
        public Parser(string FileName)
        {
            file = FileName;
        }

        private RxoStruct Parse()
        {
            RxoStruct rxoStruct = rxo_parse(file);
            return rxoStruct;
        }

        public Model ParseDictToModel()
        {
            RxoStruct rxo = Parse();
            return new Model
            {
                db_password = rxo.db_password,
                latency_ms = rxo.latency_ms,
                status_code = rxo.status_code,
                k_factor = rxo.k_factor
            };
        }
    }
}