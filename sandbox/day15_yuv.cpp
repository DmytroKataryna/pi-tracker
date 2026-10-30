#include "pi_tracker/yuv420.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <numeric>
#include <span>
#include <string>
#include <vector>

namespace
{

    struct LumaStats
    {
        double mean{};
        std::uint8_t min{};
        std::uint8_t max{};
        double clippedPercent{}; // частка пікселів на краях діапазону: <= 5 або >= 250
    };

    double mean(std::span<const std::uint8_t> plane)
    {
        const std::uint64_t sum = std::accumulate(plane.begin(), plane.end(), std::uint64_t{0});
        return static_cast<double>(sum) / static_cast<double>(plane.size());
    }

    LumaStats computeLumaStats(const pi_tracker::Yuv420View &frame)
    {
        const auto y = frame.y();

        const auto [minIt, maxIt] = std::minmax_element(y.begin(), y.end());

        auto clipped = std::count_if(
            y.begin(),
            y.end(),
            [](std::uint8_t val)
            { return val >= 250 || val <= 5; });

        return LumaStats(
            mean(y),
            *minIt,
            *maxIt,
            static_cast<double>(clipped) / static_cast<double>(y.size()) * 100.0);
    };

    // PGM (P5): найпростіший формат сірого зображення — текстовий заголовок і далі сирі байти
    void writePgm(const std::string &path, std::span<const std::uint8_t> pixels, std::size_t width, std::size_t height)
    {
        std::ofstream out{path, std::ios::binary};
        out << "P5\n"
            << width << ' ' << height << "\n255\n";
        out.write(reinterpret_cast<const char *>(pixels.data()), static_cast<std::streamsize>(pixels.size()));
    }

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cerr << "usage: day15_yuv <file.yuv> <width> <height> [frame-to-save]\n";
        return EXIT_FAILURE;
    }
    const std::string path = argv[1];
    const auto width = static_cast<std::size_t>(std::stoul(argv[2]));
    const auto height = static_cast<std::size_t>(std::stoul(argv[3]));
    const std::size_t frameToSave = argc > 4 ? std::stoul(argv[4]) : 0;

    std::ifstream file{path, std::ios::binary};
    if (!file)
    {
        std::cerr << "cannot open " << path << '\n';
        return EXIT_FAILURE;
    }

    std::vector<std::uint8_t> buffer(pi_tracker::Yuv420View::frameSize(width, height));
    std::size_t frames = 0;

    std::cout << "frame,mean_y,min_y,max_y,clipped_pct,mean_u,mean_v\n";
    while (file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(buffer.size())))
    {
        const pi_tracker::Yuv420View frame{buffer, width, height};
        const LumaStats stats = computeLumaStats(frame);
        std::cout << std::format("{},{:.1f},{},{},{:.2f},{:.1f},{:.1f}\n", frames, stats.mean, stats.min, stats.max,
                                 stats.clippedPercent, mean(frame.u()), mean(frame.v()));

        if (frames == frameToSave)
        {
            writePgm("data/day15_luma.pgm", frame.y(), width, height);
        }
        ++frames;
    }

    std::cerr << std::format("frames: {}, frame size: {} bytes\n", frames, buffer.size());
    if (file.gcount() != 0)
    {
        std::cerr << std::format("warning: {} trailing bytes — file size is not a multiple of the frame size "
                                 "(wrong width/height or rows with padding?)\n",
                                 file.gcount());
    }
}