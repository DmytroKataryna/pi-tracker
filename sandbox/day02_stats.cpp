#include <iostream>
#include <vector>
#include <algorithm> 

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const std::size_t middle = values.size() / 2;

    if (values.size() % 2 == 1) {
        return values[middle];                                // непарна кількість
    }
    return (values[middle - 1] + values[middle]) / 2.0;       // парна кількість
}

std::vector<double> readValues()
{
    std::vector<double> values;
    double v{};
    while (std::cin >> v)
    {                        // цикл триває, поки вдається прочитати число
        values.push_back(v); // додати в кінець вектора
    }
    return values;
}

double minValue(const std::vector<double> &values)
{
    double result = values[0];
    for (double v : values)
    {
        if (v < result)
        {
            result = v;
        }
    }
    return result;
}

double maxValue(const std::vector<double> &values)
{
    double result = values[0];
    for (double v : values)
    {
        if (v > result)
        {
            result = v;
        }
    }
    return result;
}

// TODO: повернути середнє арифметичне
double average(const std::vector<double> &values)
{
    double sum = 0.0;
    for (double v : values)
    {
        sum = sum + v;
    }
    return sum / static_cast<double>(values.size());
}

int countSpikes(const std::vector<double> &values, double expected, double tolerance)
{
    int result = 0;
    const double rangeMin = expected - tolerance;
    const double rangeMax = expected + tolerance;
    for (double v : values)
    {
        if (v < rangeMin || v > rangeMax)
        {
            result = result + 1;
        }
    }

    return result;
}

int main()
{
    const std::vector<double> values = readValues();

    if (values.empty())
    {
        std::cerr << "Немає даних\n";
        return 1;
    }

    std::cout << "Count:   " << values.size() << '\n';
    std::cout << "Min:     " << minValue(values) << '\n';
    std::cout << "Max:     " << maxValue(values) << '\n';
    std::cout << "Average: " << average(values) << '\n';
    std::cout << "Spikes:  " << countSpikes(values, 9.81, 0.5) << '\n';
    return 0;
}