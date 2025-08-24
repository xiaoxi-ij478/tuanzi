#ifndef CHECKRUNTHREAD_H_INCLUDED
#define CHECKRUNTHREAD_H_INCLUDED

class CCheckRunThread
{
    public:
        static bool StartThread(int &result);
        static unsigned StopThread();

    private:
        static unsigned create_flock();
        static unsigned delete_flock();

        static int lock_file_fd;
};

#endif // CHECKRUNTHREAD_H_INCLUDED
