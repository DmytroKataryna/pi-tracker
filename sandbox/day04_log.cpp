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

// Лог вимірювань IMU: зберігає вимірювання і рахує статистику
class ImuLog
{
public:
    // Розбирає рядок CSV. Коректний рядок додає до вимірювань, битий — рахує.
    bool addLine(const std::string &line);

    std::size_t size() const { return m_samples.size(); }
    int badLines() const { return m_badLines; }

    std::array<double, 3> averageAccel() const;
    double maxAbsGyro() const;
    int countGaps(std::uint32_t expectedPeriodMs) const;
    double sampleRateHz() const;

private:
    std::vector<ImuSample> m_samples; // префікс m_ означає member, поле класу
    int m_badLines{};
};

// Визначення методу поза класом: ImuLog:: означає «цей addLine належить класу ImuLog»
bool ImuLog::addLine(const std::string &line)
{
    ImuSample sample{};
    if (!parseLine(line, sample))
    {
        ++m_badLines;
        return false;
    }
    m_samples.push_back(sample);
    return true;
}

std::array<double, 3> ImuLog::averageAccel() const
{
    double sumAccelX{};
    double sumAccelY{};
    double sumAccelZ{};

    for (const ImuSample &v : m_samples)
    {
        sumAccelX = sumAccelX + v.accel.at(0);
        sumAccelY = sumAccelY + v.accel.at(1);
        sumAccelZ = sumAccelZ + v.accel.at(2);
    }

    double averageAccelX = sumAccelX / static_cast<double>(m_samples.size());
    double averageAccelY = sumAccelY / static_cast<double>(m_samples.size());
    double averageAccelZ = sumAccelZ / static_cast<double>(m_samples.size());

    return std::array{averageAccelX, averageAccelY, averageAccelZ};
}

double ImuLog::maxAbsGyro() const
{
    double result{};
    for (const ImuSample &v : m_samples)
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

int ImuLog::countGaps(std::uint32_t expectedPeriodMs) const
{
    int count{};
    for (size_t i = 1; i < m_samples.size(); i++)
    {
        const std::uint32_t dt = m_samples[i].timestampMs - m_samples[i - 1].timestampMs;
        if (dt > (expectedPeriodMs * 1.5))
        {
            count = count + 1;
        }
    }
    return count;
}

double ImuLog::sampleRateHz() const
{
    const std::uint32_t durationMs = m_samples.back().timestampMs - m_samples.front().timestampMs;
    const double durationS = durationMs / 1000.0; // .0 означає ділення в double
    return static_cast<double>(m_samples.size() - 1) / durationS;
}

void printReport(const ImuLog &log)
{
    const std::array<double, 3> avg = log.averageAccel();
    std::cout << "Samples:     " << log.size() << '\n';
    std::cout << "Bad lines:   " << log.badLines() << '\n';
    std::cout << "Avg accel:   " << avg[0] << ", " << avg[1] << ", " << avg[2] << '\n';
    std::cout << "Max |gyro|:  " << log.maxAbsGyro() << '\n';
    std::cout << "Gaps:        " << log.countGaps(10) << '\n';
    std::cout << "Sample rate: " << log.sampleRateHz() << " Hz\n";
}

int main()
{
    ImuLog log;
    std::string line;
    std::getline(std::cin, line); // заголовок

    while (std::getline(std::cin, line))
    {
        log.addLine(line);
    }

    if (log.size() < 2)
    {
        std::cerr << "Замало даних\n";
        return 1;
    }
    printReport(log);
}