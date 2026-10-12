#include <fstream>
#include <iostream>

int main() {
    // Ядро Linux показує температуру як файл (у тисячних °C)
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    if (!file) {
        std::cerr << "Не вдалося відкрити файл температури\n";
        return 1;
    }

    int milli_celsius = 0;
    file >> milli_celsius;
    std::cout << "CPU temperature: " << milli_celsius / 1000.0 << " °C\n";
    return 0;
}