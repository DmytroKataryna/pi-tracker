#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

double median(std::vector<double> values)
{
    auto middle = values.begin() + (values.size() / 2);
    std::nth_element(values.begin(), middle, values.end());
    return *middle;
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
    auto [min_it, max_it] = std::ranges::minmax_element(values);
    return *min_it;
}

double maxValue(const std::vector<double> &values)
{
    auto [min_it, max_it] = std::ranges::minmax_element(values);
    return *max_it;
}

double average(const std::vector<double> &values)
{
    auto acc = std::accumulate(values.begin(), values.end(), 0.0);
    return acc / static_cast<double>(values.size());
}

int countSpikes(const std::vector<double> &values, double expected, double tolerance)
{
    const auto spikes = std::ranges::count_if(
        values, [expected, tolerance](double v)
        { return std::abs(v - expected) > tolerance; });

    return static_cast<int>(spikes);
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
    std::cout << "Median:  " << median(values) << '\n';
    return 0;
}