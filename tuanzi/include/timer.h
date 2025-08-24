#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

// std::thread-like interface

class timer
{
    public:
        timer() = default;

        timer(const timer &) = delete;
        timer &operator=(const timer &) = delete;

        timer(timer &&other) :
            start_delay(std::move(other.start_delay)),
            interval(std::move(other.interval)),
            final_func(std::move(other.final_func)),
            timer_thread(std::move(other.timer_thread)),
            running_var(other.running_var.load())
        {}

        timer &operator=(timer &&other)
        {
            start_delay = std::move(other.start_delay);
            interval = std::move(other.interval);
            final_func = std::move(other.final_func);
            timer_thread = std::move(other.timer_thread);
            running_var = other.running_var.load();
            return *this;
        }

        template <typename Fn, typename... Args>
        timer(
            std::chrono::microseconds start_delay,
            std::chrono::microseconds interval,
            Fn &&func,
            Args &&...args // *NOPAD*
        ) :
            start_delay(start_delay),
            interval(interval),
            final_func(std::bind(func, std::forward<Args>(args)...)),
            timer_thread(std::thread(std::mem_fn(&timer::loop_func), this)),
            running_var(true) {}

        ~timer()
        {
            running_var = false;

            if (timer_thread.joinable())
                timer_thread.join();
        }

        bool running() const
        {
            return running_var;
        }

        operator bool() const
        {
            return running_var;
        }

    private:
        void loop_func()
        {
            std::this_thread::sleep_for(start_delay);

            while (running_var) {
                final_func();
                std::this_thread::sleep_for(interval);
            }
        }

        std::chrono::microseconds start_delay;
        std::chrono::microseconds interval;
        std::function<void()> final_func;
        std::thread timer_thread;
        std::atomic_bool running_var;
};

#endif // TIMER_H_INCLUDED
