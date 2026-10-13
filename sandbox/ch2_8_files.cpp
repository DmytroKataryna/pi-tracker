#include <iostream>

int getInteger();

int main() {
    int x = getInteger();
    std::cout << "X: " << x << std::endl;
    return 0;
}