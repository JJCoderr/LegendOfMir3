#pragma once

#include <windows.h>
#include <xaudio2.h>

#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace mir3_cui {

struct AudioOptions {
    bool music = true;
    bool effects = true;
    bool reversePan = false;
    int musicLevel = 0;
    int effectsLevel = 0;

    static constexpr int KnobFromLevel(int level) { return 9 * (level + 100) / 5; }
    static constexpr int LevelFromKnob(int offset) { return (5 * offset - 900) / 9; }
    static constexpr int KnobOffset(int x) { return x < 288 ? 0 : x > 468 ? 185 : x - 288; }
    static constexpr int TrackStep(int offset, int x)
    {
        if (x <= 266 || x >= 487) return offset;
        if (x > offset + 292 || x > 477) return offset + 9 > 180 ? 185 : offset + 9;
        if (x < offset + 276 || x < 275) return offset < 9 ? 0 : offset - 9;
        return offset;
    }

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
    void PauseMusic(bool pause);
    void PlayEffect(const wchar_t* path, const AudioOptions& options);
    bool PlaySceneEffect(std::int64_t actor, const wchar_t* path, int volume, int pan, const AudioOptions& options);
    bool SceneEffectPlaying(std::int64_t actor) const;
    void MoveSceneEffect(std::int64_t actor,int volume,int pan,const AudioOptions& options);
    void StopSceneEffect(std::int64_t actor);
    void ClearSceneEffects(bool keepLocal = false);
    void PlayMovieAudio(const std::vector<unsigned char>& samples, int channels, int rate, int bits,
                        const AudioOptions& options);
    void StopMovieAudio();
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
    Track movie_;
    struct SceneTrack {
        Track audio;
        int volume = 0, pan = 0;
    };
    std::map<std::int64_t, SceneTrack> scene_;
    bool musicPaused_ = false;
    std::wstring musicPath_;
    std::string error_;

    bool Open();
    bool LoadWave(const wchar_t* path, Track& track);
    bool Start(Track& track, bool loop, const AudioOptions& options);
    void ApplySceneTrack(const SceneTrack& sound,const AudioOptions& options);
};

} // namespace mir3_cui
