#ifndef WAITHANDLE_H_INCLUDED
#define WAITHANDLE_H_INCLUDED

struct WAIT_HANDLE {
    std::atomic_bool signal;
    std::condition_variable condition;
    std::recursive_timed_mutex mutex;
};

#endif // WAITHANDLE_H_INCLUDED
