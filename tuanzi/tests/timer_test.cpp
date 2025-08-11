#include "all.h"
#include "timer.h"

void r(int i)
{
    std::cout << i << std::endl;
}

int main()
{
    using namespace std::literals;
    Timer t1;
    t1 = Timer(0s, 1ms, [] { std::cout << "1" << std::endl; });
    t1.start();
    std::this_thread::sleep_for(1s);
    t1.stop();
}
