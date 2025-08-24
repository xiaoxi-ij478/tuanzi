#include "all.h"
#include "timer.h"

void r(int i)
{
    std::cout << i << std::endl;
}

int main()
{
    using namespace std::literals;
    timer t1(0s, 100ms, r, 2);
    std::this_thread::sleep_for(500ms);
}
