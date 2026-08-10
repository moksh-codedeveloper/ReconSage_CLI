using System.Runtime.InteropServices;

namespace ResponseBodyStruct
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct BodyStruct
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 4096)]
        public string captured_body;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 3072)]
        public string domain;
        public int statusCode;
    }

    public class ScanResponseBodyModel
    {
        public string target{set;get;} = string.Empty;
        public string bodyResponse{set;get;} = string.Empty;
        public int statusCode{set;get;}
    }

    public class MainScanResponseBodyModel
    {
        public List<ScanResponseBodyModel> Result{get; set;} = new();
    }
}