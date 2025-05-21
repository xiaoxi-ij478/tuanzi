#ifndef LOGFILE_H
#define LOGFILE_H

// most of the functions in this class in the original implementation
// are just stubs
// maybe because they are disabled in the production release
// but I choose to implement them
class CLogFile
{
    public:
        CLogFile();

        [[gnu::format(printf, 2, 3)]]
        void AppendText(const char *format, ...);
        void AppendText_V(const char *format, va_list va);
        // TODO: prio arg is unknown
        void CreateLogFile_S(const std::string &filename, int prio);
        void HexPrinter(const unsigned char *arr, unsigned len);
        void WriteString(const char *str);

        static void LogToFile(
            const char *log_msg,
            const char *filename,
            bool print_time,
            bool print_crlf
        );

    private:
        void CreateLogFile(const char *filename, int prio);

        static void GetTimeString(char *dst);

        std::ofstream ofs;
        int prio;
        std::streamsize ofs_orig_precision;
};

#endif // LOGFILE_H
