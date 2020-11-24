#include <iostream>

int main() {
    int* ptr = new int;

    std::cout << "ptr: " << ptr << "\nuninitialized int: " << *ptr << std::endl;

    *ptr = 1;

    std::cout << "value was set to 1: " << *ptr << std::endl;
    delete ptr;
    return 0;
}
