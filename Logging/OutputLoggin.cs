namespace ReconSageLogger
{
    public static class Logger
    {
        public static void Info(string message)
        {
            Console.ForegroundColor = ConsoleColor.Blue;
            Console.Write("[INFO] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Warn(string message)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write("[WARN] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Error(string message)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("[ERROR] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Success(string message)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.Write("[SUCCESS] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Scan(string message)
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.Write("[SCAN] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Rotate(string message)
        {
            Console.ForegroundColor = ConsoleColor.DarkYellow;
            Console.Write("[ROTATE] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }

        public static void Done(string message)
        {
            Console.ForegroundColor = ConsoleColor.Gray;
            Console.Write("[DONE] ");
            Console.WriteLine(message);
            Console.ResetColor();
        }
    }
}