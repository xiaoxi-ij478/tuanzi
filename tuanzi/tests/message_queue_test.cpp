#include "all.h"
#include "message_queue.h"

struct test: base_message {
    test(unsigned long type, int a, std::string b): base_message(type),a(a), b(b) {}
    int a;
    std::string b;
};

message_queue que;

void getter()
{
    for(int i=0;i<2;i++) {
        std::unique_ptr<struct test> ta = que.get<struct test>();
        std::cout << ta->a << ta->b << std::endl;
    }
}

int main()
{
    std::thread a(getter);
    que.put(std::make_unique<struct test>(0, 1, "abc"));
    que.put(std::make_unique<struct test>(0, 2, "def"));
    a.join();
}
