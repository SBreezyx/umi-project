#include <iostream>
#include <thread>

auto main() -> int {
    std::cout << "The number of supported threads is: " << std::thread::hardware_concurrency() << std::endl;
}