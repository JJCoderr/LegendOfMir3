#pragma once
// MIR3 47C430 / Godot poll_connection: a native Yes/No prompt, then 2000 ms.
struct ConnectionRetry {
    enum class Phase {Idle,Prompt,Waiting,Quit};
    static constexpr DWORD Delay=2000;
    static constexpr int Message=132;
    static constexpr const char* Title="Legend Of Mir 3";
    Phase phase=Phase::Idle;
    DWORD started=0;
    void Fail() {phase=Phase::Prompt;}
    void Answer(bool yes,DWORD now) {phase=yes?Phase::Waiting:Phase::Quit;started=now;}
    bool Due(DWORD now) const {return phase==Phase::Waiting && DWORD(now-started)>=Delay;}
    bool Blocks() const {return phase!=Phase::Idle;}
};
