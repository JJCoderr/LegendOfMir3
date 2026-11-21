#include "CuiAudio.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace mir3_cui {
namespace {

int NormalizeLevel(int value)
{
    return value >= -100 && value <= 0 ? value : 0;
}

float Volume(bool enabled, int level)
{
    // Godot maps each level to 0.4 dB. XAudio2 expects linear amplitude.
    return enabled ? static_cast<float>(std::pow(10.0, (level >= -100 && level <= 2 ? level : 0) / 50.0)) : 0.0f;
}

std::uint32_t ReadU32(const unsigned char* bytes)
{
    return bytes[0] | (std::uint32_t(bytes[1]) << 8) |
        (std::uint32_t(bytes[2]) << 16) | (std::uint32_t(bytes[3]) << 24);
}

WORD ReadU16(const unsigned char* bytes)
{
    return static_cast<WORD>(bytes[0] | (bytes[1] << 8));
}

} // namespace

void AudioOptions::Load(const wchar_t* path)
{
    const auto read = [&](const wchar_t* key, int fallback) {
        const auto legacy = static_cast<int>(GetPrivateProfileIntW(L"Audio", key, fallback, path));
        return static_cast<int>(GetPrivateProfileIntW(L"Options", key, legacy, path));
    };
    music = read(L"BGM", 1) != 0;
    effects = read(L"EffectSound", 1) != 0;
    reversePan = read(L"ReversePan", 0) == 1;
    musicLevel = NormalizeLevel(read(L"BGMLevel", 0));
    effectsLevel = NormalizeLevel(read(L"EffectSoundLevel", 0));
}

bool AudioOptions::Save(const wchar_t* path) const
{
    std::wstring section;
    bool saved = true;
    const auto append = [&](const wchar_t* key, int value) {
        const auto text = std::to_wstring(value);
        saved = WritePrivateProfileStringW(L"Options", key, text.c_str(), path) != 0 && saved;
        section += key;
        section += '=';
        section += text;
        section += L'\0';
    };
    append(L"BGM", music);
    append(L"EffectSound", effects);
    append(L"ReversePan", reversePan);
    append(L"BGMLevel", musicLevel);
    append(L"EffectSoundLevel", effectsLevel);
    return WritePrivateProfileSectionW(L"Audio", section.c_str(), path) != 0 && saved;
}

void AudioPlayer::Track::Clear()
{
    if (voice) voice->DestroyVoice();
    voice = nullptr;
    samples.clear();
    format = {};
}

AudioPlayer::~AudioPlayer()
{
    music_.Clear();
    effect_.Clear();
    movie_.Clear();
    ClearSceneEffects();
    if (output_) output_->DestroyVoice();
    if (engine_) engine_->Release();
}

bool AudioPlayer::Open()
{
    if (output_) return true;
    if (!engine_ && FAILED(XAudio2Create(&engine_))) {
        error_ = "Could not start audio.";
        return false;
    }
    if (FAILED(engine_->CreateMasteringVoice(&output_, 2))) {
        error_ = "No stereo audio output is available.";
        return false;
    }
    return true;
}

bool AudioPlayer::LoadWave(const wchar_t* path, Track& track)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;
    constexpr std::streamoff limit = 64 * 1024 * 1024;
    const auto length = file.tellg();
    if (length < 12 || length > limit) return false;
    file.seekg(0);
    std::vector<unsigned char> bytes(static_cast<std::size_t>(length));
    if (!file.read(reinterpret_cast<char*>(bytes.data()), length) ||
        std::memcmp(bytes.data(), "RIFF", 4) != 0 || std::memcmp(bytes.data() + 8, "WAVE", 4) != 0)
        return false;
    const auto riffSize = ReadU32(bytes.data() + 4);
    if (riffSize < 4 || riffSize > bytes.size() - 8) return false;
    const std::size_t end = 8 + riffSize;

    const unsigned char* format = nullptr;
    const unsigned char* samples = nullptr;
    std::uint32_t sampleBytes = 0;
    for (std::size_t offset = 12; offset + 8 <= end;) {
        const auto* chunk = bytes.data() + offset;
        const auto chunkSize = ReadU32(chunk + 4);
        offset += 8;
        if (chunkSize > end - offset || (chunkSize % 2 && chunkSize == end - offset)) return false;
        if (std::memcmp(chunk, "fmt ", 4) == 0 && chunkSize >= 16) format = bytes.data() + offset;
        if (std::memcmp(chunk, "data", 4) == 0) {
            samples = bytes.data() + offset;
            sampleBytes = chunkSize;
        }
        offset += chunkSize + chunkSize % 2;
    }
    if (!format || !samples || !sampleBytes) return false;
    WAVEFORMATEX wave = {};
    wave.wFormatTag = ReadU16(format);
    wave.nChannels = ReadU16(format + 2);
    wave.nSamplesPerSec = ReadU32(format + 4);
    wave.nAvgBytesPerSec = ReadU32(format + 8);
    wave.nBlockAlign = ReadU16(format + 12);
    wave.wBitsPerSample = ReadU16(format + 14);
    if (wave.wFormatTag != WAVE_FORMAT_PCM || (wave.nChannels != 1 && wave.nChannels != 2) ||
        wave.nSamplesPerSec < XAUDIO2_MIN_SAMPLE_RATE || wave.nSamplesPerSec > XAUDIO2_MAX_SAMPLE_RATE ||
        (wave.wBitsPerSample != 8 && wave.wBitsPerSample != 16) ||
        wave.nBlockAlign != wave.nChannels * (wave.wBitsPerSample / 8) ||
        wave.nAvgBytesPerSec != wave.nSamplesPerSec * wave.nBlockAlign ||
        sampleBytes % wave.nBlockAlign)
        return false;

    track.format = wave;
    track.samples.assign(samples, samples + sampleBytes);
    return true;
}

bool AudioPlayer::Start(Track& track, bool loop, const AudioOptions& options)
{
    if (!Open()) {
        track.Clear();
        return false;
    }
    if (FAILED(engine_->CreateSourceVoice(&track.voice, &track.format))) {
        error_ = "Could not open the audio track.";
        track.Clear();
        return false;
    }
    XAUDIO2_BUFFER buffer = {};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(track.samples.size());
    buffer.pAudioData = track.samples.data();
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
    Apply(options);
    if (FAILED(track.voice->SubmitSourceBuffer(&buffer)) ||
        ((!loop || !musicPaused_) && FAILED(track.voice->Start()))) {
        error_ = "Could not play the audio track.";
        track.Clear();
        return false;
    }
    return true;
}

void AudioPlayer::SetMusic(const wchar_t* path, const AudioOptions& options)
{
    if (musicPath_ == path) return;
    musicPath_ = path;
    music_.Clear();
    error_.clear();
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) return;
    if (!LoadWave(path, music_)) {
        error_ = "Background music requires a mono or stereo PCM WAV file.";
        return;
    }
    Start(music_, true, options);
}

void AudioPlayer::PlayEffect(const wchar_t* path, const AudioOptions& options)
{
    if (!options.effects) {
        error_ = "Sound effects are disabled.";
        return;
    }
    effect_.Clear();
    if (!LoadWave(path, effect_)) {
        error_ = "Select a mono or stereo PCM WAV file for the sound effect.";
        return;
    }
    error_.clear();
    Start(effect_, false, options);
}

bool AudioPlayer::PlaySceneEffect(std::uint32_t actor, const wchar_t* path, int volume, int pan,
                                  const AudioOptions& options)
{
    if (!options.effects || volume + options.effectsLevel * 40 <= -10000) return false;
    for (auto it=scene_.begin();it!=scene_.end();) {
        XAUDIO2_VOICE_STATE state{};
        if (it->second.audio.voice) it->second.audio.voice->GetState(&state,XAUDIO2_VOICE_NOSAMPLESPLAYED);
        if (state.BuffersQueued) ++it;
        else {
            it->second.audio.Clear();
            it=scene_.erase(it);
        }
    }
    auto& sound=scene_[actor];
    sound.audio.Clear();
    sound.volume=volume;
    sound.pan=pan;
    if (LoadWave(path,sound.audio) && Start(sound.audio,false,options)) return true;
    scene_.erase(actor);
    return false;
}

void AudioPlayer::StopSceneEffect(std::uint32_t actor)
{
    const auto found=scene_.find(actor);
    if (found==scene_.end()) return;
    found->second.audio.Clear();
    scene_.erase(found);
}

void AudioPlayer::ClearSceneEffects(bool keepLocal)
{
    for (auto it=scene_.begin();it!=scene_.end();) {
        if (keepLocal && it->first==0) { ++it; continue; }
        it->second.audio.Clear();
        it=scene_.erase(it);
    }
}

void AudioPlayer::PauseMusic(bool pause)
{
    if (musicPaused_ == pause) return;
    musicPaused_ = pause;
    if (!music_.voice) return;
    if (pause) music_.voice->Stop();
    else music_.voice->Start();
}

void AudioPlayer::PlayMovieAudio(const std::vector<unsigned char>& samples, int channels, int rate, int bits,
                                 const AudioOptions& options)
{
    StopMovieAudio();
    if (!options.effects || samples.empty() || (channels != 1 && channels != 2) ||
        (bits != 8 && bits != 16) || rate < XAUDIO2_MIN_SAMPLE_RATE || rate > XAUDIO2_MAX_SAMPLE_RATE)
        return;
    const auto align = static_cast<WORD>(channels * (bits / 8));
    if (samples.size() % align || samples.size() > XAUDIO2_MAX_BUFFER_BYTES) return;
    movie_.format = {WAVE_FORMAT_PCM, static_cast<WORD>(channels), static_cast<DWORD>(rate),
                     static_cast<DWORD>(rate * align), align, static_cast<WORD>(bits), 0};
    movie_.samples = samples;
    Start(movie_, false, options);
}

void AudioPlayer::StopMovieAudio()
{
    movie_.Clear();
}

void AudioPlayer::Apply(const AudioOptions& options)
{
    if (music_.voice) music_.voice->SetVolume(Volume(options.music, options.musicLevel));
    const float normal = options.reversePan ? 0.0f : 1.0f;
    const float swapped = options.reversePan ? 1.0f : 0.0f;
    const float matrix[] = {normal, swapped, swapped, normal};
    for (auto* track : {&effect_, &movie_}) {
        if (!track->voice) continue;
        track->voice->SetVolume(Volume(options.effects, options.effectsLevel));
        if (track->format.nChannels == 2) track->voice->SetOutputMatrix(output_, 2, 2, matrix);
    }
    for (auto& [actor,sound]:scene_) {
        const auto& track=sound.audio;
        if (!track.voice) continue;
        const float attenuation=static_cast<float>(std::pow(10.0,sound.volume/2000.0));
        track.voice->SetVolume(Volume(options.effects,options.effectsLevel)*attenuation);
        const double pan=(options.reversePan ? -sound.pan : sound.pan)/2000.0;
        const float left=static_cast<float>(pan>0 ? std::pow(10.0,-pan) : 1.0);
        const float right=static_cast<float>(pan<0 ? std::pow(10.0,pan) : 1.0);
        const float panMatrix[]{left,right};
        if (track.format.nChannels==1) track.voice->SetOutputMatrix(output_,1,2,panMatrix);
        else {
            const float stereo[]{left,0,0,right};
            track.voice->SetOutputMatrix(output_,2,2,stereo);
        }
    }
}

} // namespace mir3_cui
