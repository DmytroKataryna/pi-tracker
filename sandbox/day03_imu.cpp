#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Одне вимірювання IMU
struct ImuSample
{
    std::uint32_t timestampMs{};   // час від старту, мс
    std::array<double, 3> accel{}; // прискорення X, Y, Z, м/с²
    std::array<double, 3> gyro{};  // кутова швидкість X, Y, Z, °/с
};

bool parseLine(const std::string &line, ImuSample &out)
{
    std::istringstream ss{line}; // потік, який читає з рядка, як std::cin з клавіатури

    std::uint32_t timestamp{};
    if (!(ss >> timestamp))
    {
        return false; // перше поле — не число
    }

    std::array<double, 6> values{};
    for (double &value : values)
    { // & — щоб записувати в елементи масиву, а не в їхні копії
        char comma{};
        if (!(ss >> comma >> value) || comma != ',')
        {
            return false; // немає коми або числа — рядок битий
        }
    }

    out.timestampMs = timestamp;
    out.accel = {values[0], values[1], values[2]};
    out.gyro = {values[3], values[4], values[5]};
    return true;
}

std::array<double, 3> averageAccel(const std::vector<ImuSample> &samples)
{
    double sumAccelX{};
    double sumAccelY{};
    double sumAccelZ{};

    for (const ImuSample &v : samples)
    {
        sumAccelX = sumAccelX + v.accel.at(0);
        sumAccelY = sumAccelY + v.accel.at(1);
        sumAccelZ = sumAccelZ + v.accel.at(2);
    }

    double averageAccelX = sumAccelX / static_cast<double>(samples.size());
    double averageAccelY = sumAccelY / static_cast<double>(samples.size());
    double averageAccelZ = sumAccelZ / static_cast<double>(samples.size());

    return std::array{averageAccelX, averageAccelY, averageAccelZ};
}

double maxAbsGyro(const std::vector<ImuSample> &samples)
{
    double result{};
    for (const ImuSample &v : samples)
    {
        double gyroX = std::abs(v.gyro.at(0));
        double gyroY = std::abs(v.gyro.at(1));
        double gyroZ = std::abs(v.gyro.at(2));
        if (gyroX > result)
        {
            result = gyroX;
        }

        if (gyroY > result)
        {
            result = gyroY;
        }

        if (gyroZ > result)
        {
            result = gyroZ;
        }
    }
    return result;
}

int countGaps(const std::vector<ImuSample> &samples, std::uint32_t expectedPeriodMs)
{
    int count{};
    for (size_t i = 1; i < samples.size(); i++)
    {
        const std::uint32_t dt = samples[i].timestampMs - samples[i - 1].timestampMs;
        if (dt > (expectedPeriodMs * 1.5))
        {
            count = count + 1;
        }
    }
    return count;
}

double sampleRateHz(const std::vector<ImuSample> &samples)
{
    const std::uint32_t durationMs = samples.back().timestampMs - samples.front().timestampMs;
    const double durationS = durationMs / 1000.0; // .0 означає ділення в double
    return static_cast<double>(samples.size() - 1) / durationS;
}

int main()
{
    std::vector<ImuSample> samples;
    int badLines = 0;

    std::string line;
    std::getline(std::cin, line); // пропускаємо заголовок CSV

    while (std::getline(std::cin, line))
    {
        ImuSample sample{};
        if (parseLine(line, sample))
        {
            samples.push_back(sample);
        }
        else
        {
            ++badLines;
        }
    }

    if (samples.size() < 2)
    {
        std::cerr << "Замало даних\n";
        return 1;
    }

    const std::array<double, 3> avg = averageAccel(samples);
    std::cout << "Samples:     " << samples.size() << '\n';
    std::cout << "Bad lines:   " << badLines << '\n';
    std::cout << "Avg accel:   " << avg[0] << ", " << avg[1] << ", " << avg[2] << '\n';
    std::cout << "Max |gyro|:  " << maxAbsGyro(samples) << '\n';
    std::cout << "Gaps:        " << countGaps(samples, 10) << '\n';
    std::cout << "Sample rate: " << sampleRateHz(samples) << " Hz\n";
}