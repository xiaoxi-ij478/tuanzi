#ifndef LOGFILE_H
#define LOGFILE_H

class CLogFile
{
    public:
        CLogFile();

        void AppendText(const char *format, ...);
        void AppendText_V(const char *format, va_list va);
        // TODO: prio arg is unknown
        void CreateLogFile_S(const std::string &filename, int prio);
        void HexPrinter(const unsigned char *arr, unsigned int len);
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
};

#endif // LOGFILE_H
