#ifndef MESSAGE_QUEUE_H_INCLUDED
#define MESSAGE_QUEUE_H_INCLUDED

struct base_message {
    base_message(unsigned long type) : type(type) {}
    virtual ~base_message() {}

    unsigned long type;
};

class message_queue
{
    public:
        unsigned long peek_type()
        {
            wait();
            return messages.front()->type;
        }

        void wait()
        {
            std::unique_lock lock(mutex);
            condition.wait(lock, [this] { return !messages.empty(); });
        }

        template<typename T>
        std::unique_ptr<T> get()
        {
            wait();
            T *ptr = dynamic_cast<T *>(messages.front().get());

            // the pointer can't be null because it's checked in put()
            // so if it's null the conversion failed
            if (!ptr)
                throw std::bad_cast();

            messages.front().release();
            std::unique_ptr<T> final_message = std::unique_ptr<T>(ptr);
            messages.pop();
            return final_message;
        }

        template<typename T>
        void put(std::unique_ptr<T> message)
        {
            if (!message)
                return;

            messages.push(
                std::unique_ptr<struct base_message>(
                    static_cast<struct base_message *>(message.release())
                )
            );
            condition.notify_one();
        }

    private:
        std::queue<std::unique_ptr<struct base_message>> messages;
        std::mutex mutex;
        std::condition_variable condition;
};

#endif // MESSAGE_QUEUE_H_INCLUDED
