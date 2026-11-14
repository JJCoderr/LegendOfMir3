#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>
namespace mir3 {
class LegacyMovie {
    struct State;
    std::unique_ptr<State> state;
public:
    struct Audio { int channels=0,rate=0,bits=0;std::vector<std::uint8_t> pcm; };
    LegacyMovie();
    ~LegacyMovie();
    LegacyMovie(const LegacyMovie&)=delete;
    LegacyMovie& operator=(const LegacyMovie&)=delete;
    bool open(const std::filesystem::path& path,bool reverse=false);
    void close();
    bool advance(double milliseconds);
    bool playing() const;
    int width() const;
    int height() const;
    const std::vector<std::uint8_t>& pixels() const;
    const Audio& audio() const;
};
}
