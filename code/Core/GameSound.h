#pragma warning(disable:4996)
#pragma once

namespace GameSound
{
    enum class SE { Start, Cursor, Push, Pop, Hand, Good, Bad, Result, MAX };
    enum class BGM { Title, Game, Ending, MAX };
    //起動時に一度。SE は WAV(Dm2008)、BGM は mp3(SoundLib/DirectShow)
    void LoadAll();
    //毎フレーム。BGM のループ監視
    void UpDate();
    void Release();
    void Play(SE se_);
    void PlayBgm(BGM bgm_);
    void StopBgm();
}