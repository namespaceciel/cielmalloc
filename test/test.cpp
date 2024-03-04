#include <ciel/malloc.h>
#include <ciel/span.h>

#include <iostream>

int main() {
    int* ptr = static_cast<int*>(ciel::malloc(sizeof(int)));
    int* ptr2 = static_cast<int*>(ciel::malloc(sizeof(int)));
    int* ptr3 = static_cast<int*>(ciel::malloc(sizeof(int)));

    std::cout << ciel::get_span(ptr) << '\n'
              << ptr << '\n'
              << ptr2 << '\n'
              << ptr3;

    ciel::free(ptr);
    ciel::free(ptr2);
    ciel::free(ptr3);
}