#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

class Timer
{
    public:
        Timer() : stopped(true) {}

        Timer(const Timer&) = delete;

        Timer& operator=(const Timer&) = delete;

        Timer(Timer&& other) :
            start_delay(std::move(other.start_delay)),
            interval(std::move(other.interval)),
            final_func(std::move(other.final_func)),
            timer_thread(std::move(other.timer_thread)),
            stopped(other.stopped.load()) {}

        Timer& operator=(Timer&& other) {
            start_delay = std::move(other.start_delay);
            interval = std::move(other.interval);
            final_func = std::move(other.final_func);
            timer_thread = std::move(other.timer_thread);
            stopped = other.stopped.load();
            return *this;
        }

        template <typename Fn, typename... Args>
        Timer(
            std::chrono::microseconds start_delay,
            std::chrono::microseconds interval,
            Fn&& func,
            Args&& ... args
        ) :
            start_delay(start_delay),
            interval(interval),
            final_func(std::bind(func, std::forward<Args>(args)...)),
            stopped(true) {}

        ~Timer() {
            stop();
        }

        void start() {
            if (!stopped)
                return;

            stopped = false;
            timer_thread = std::thread(std::mem_fn(&Timer::loop_func), this);
        }

        void stop() {
            if (stopped)
                return;

            stopped = true;
            timer_thread.join();
        }

    private:
        void loop_func() {
            std::this_thread::sleep_for(start_delay);

            while (!stopped) {
                final_func();
                std::this_thread::sleep_for(interval);
            }
        }

        std::chrono::microseconds start_delay;
        std::chrono::microseconds interval;
        std::function<void()> final_func;
        std::thread timer_thread;
        std::atomic<bool> stopped;
};

#endif // TIMER_H_INCLUDED
