#include <iostream>
void counter() {
    static int count = 0; // 只在第一次调用时初始化
    count++;
    std::cout << "Count: " << count << std::endl;
}
int main() {
    counter(); // Count: 1
    counter(); // Count: 2
}