#include "changelanguage.h"
#include "logfile.h"
#include "util.h"
#include "signals.h"

void set_signals()
{
    signal(SIGINT, sig_quit);
    signal(SIGQUIT, sig_quit);
    signal(SIGTSTP, sig_tstp);
    signal(SIGHUP, sig_quit);
    signal(SIGTERM, sig_quit);
}

void release_signals()
{
    sigrelse(SIGINT);
    sigrelse(SIGQUIT);
    sigrelse(SIGTSTP);
    sigrelse(SIGHUP);
    sigrelse(SIGTERM);
}

void hold_signals()
{
    sighold(SIGINT);
    sighold(SIGQUIT);
    sighold(SIGTSTP);
    sighold(SIGHUP);
    sighold(SIGTERM);
}

void sig_quit(int)
{
    if (!do_quit())
        CLogFile::LogToFile(
            CChangeLanguage::Instance().LoadString(2051).c_str(),
            g_runLogFile.c_str(),
            true, true
        );

    exit(0);
}

void sig_tstp(int)
{}
