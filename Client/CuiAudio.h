#pragma once

#include <windows.h>
#include <xaudio2.h>

#include <string>
#include <vector>

namespace mir3_cui {

struct AudioOptions {
    bool music = true;
    bool effects = true;
    bool reversePan = false;
    int musicLevel = 0;
    int effectsLevel = 0;

    void Load(const wchar_t* path);
    bool Save(const wchar_t* path) const;
};

class AudioPlayer {
public:
    AudioPlayer() = default;
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;
    ~AudioPlayer();

    void SetMusic(const wchar_t* path, const AudioOptions& options);
    void PlayEffect(const wchar_t* path, const AudioOptions& options);
    void Apply(const AudioOptions& options);
    const std::string& Error() const { return error_; }

private:
    struct Track {
        IXAudio2SourceVoice* voice = nullptr;
        std::vector<unsigned char> samples;
        WAVEFORMATEX format = {};
        void Clear();
    };

    IXAudio2* engine_ = nullptr;
    IXAudio2MasteringVoice* output_ = nullptr;
    Track music_;
    Track effect_;
    std::wstring musicPath_;
    std::string error_;

    bool Open();
    bool LoadWave(const wchar_t* path, Track& track);
    bool Start(Track& track, bool loop, const AudioOptions& options);
};

} // namespace mir3_cui
