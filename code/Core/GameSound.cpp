#include "MyPG.h"
#include "GameSound.h"
#include "sound.h"
#include <fstream>

namespace GameSound
{
    const long SE_VOLUME = -600;
    const int  BGM_VOLUME = 88;             //SoundLib は 0-100(内部で -10000+v*100 のデシベルになる)
    const char* SE_FILES[(int)SE::MAX] = {
            "./data/sound/se_start.wav",  "./data/sound/se_cursor.wav",
            "./data/sound/se_push.wav",   "./data/sound/se_pop.wav",
            "./data/sound/se_hand.wav",   "./data/sound/se_good.wav",
            "./data/sound/se_bad.wav",    "./data/sound/se_result.wav"
    };
    //BGM は SoundLib(DirectShow) が扱う。資源名は文字列なので、この表の中だけに閉じ込める
    const char* BGM_NAMES[(int)BGM::MAX] = { "bgm_title", "bgm_game", "bgm_ending" };
    const char* BGM_FILES[(int)BGM::MAX] = {
            "./data/sound/bgm_title.mp3", "./data/sound/bgm_game.mp3",
            "./data/sound/bgm_ending.mp3"
    };

    DM::Sound::SP   g_se[(int)SE::MAX];
    bool                    g_bgmLive[(int)BGM::MAX] = {};  //鳴らせる BGM だけ true
    BGM                             g_current = BGM::MAX;                   //MAX は「何も鳴っていない」
    bool                    g_loaded = false;

    //ファイルがそこに在るか。無ければ SoundLib を呼ばない(assert が出ない)
    bool Exists(const char* path_)
    {
        ifstream fin(path_, ios::binary);
        return fin.is_open();
    }

    //音声機器やファイルが無くてもゲームを続ける
    void LoadAll()
    {
        if (g_loaded) { return; }
        g_loaded = true;

        //SE は Dm2008(WAV 専用)。dmi が無ければ SE だけ諦める
        if (nullptr != ge && nullptr != ge->dmi) {
            for (int i = 0; i < (int)SE::MAX; ++i) {
                g_se[i] = DM::Sound::CreateSE(SE_FILES[i]);
            }
        }

        //BGM は SoundLib(DirectShow)。mp3 が読める
        bgm::Init();            //CoInitialize
        for (int i = 0; i < (int)BGM::MAX; ++i) {
            if (!Exists(BGM_FILES[i])) { continue; }        //g_bgmLive[i] は false のまま
            bgm::LoadFile(BGM_NAMES[i], BGM_FILES[i]);
            g_bgmLive[i] = true;
        }

#ifdef _DEBUG
        //音は目で確かめられない。これが唯一の手がかりになる
        {
            ofstream fout("_sound_report.txt");
            for (int i = 0; i < (int)SE::MAX; ++i) {
                fout << (nullptr != g_se[i] ? "OK      " : "FAILED  ") << SE_FILES[i] << endl;
            }
            for (int i = 0; i < (int)BGM::MAX; ++i) {
                fout << (g_bgmLive[i] ? "OK      " : "MISSING ") << BGM_FILES[i] << endl;
            }
        }
#endif
    }

    //毎フレーム。SoundLib が再生位置を見てループさせる
    void UpDate()
    {
        if (!g_loaded) { return; }
        bgm::EndCheck();
    }

    void Play(SE se_)
    {
        const int i = (int)se_;
        if (0 > i || (int)SE::MAX <= i || nullptr == g_se[i]) { return; }
        g_se[i]->SetVolume(SE_VOLUME);
        g_se[i]->Play_Normal(false);
    }

    void PlayBgm(BGM bgm_)
    {
        const int i = (int)bgm_;
        if (0 > i || (int)BGM::MAX <= i) { return; }
        if (g_current == bgm_) { return; }              //同じ曲の鳴らし直しはしない
        StopBgm();
        if (!g_bgmLive[i]) { return; }

        //SoundLib の表に無い資源名は .at() が例外を投げる。ここで止める
        try {
            bgm::VolumeControl(BGM_NAMES[i], BGM_VOLUME);
            bgm::Play(BGM_NAMES[i]);
            g_current = bgm_;
        }
        catch (...) {
            g_bgmLive[i] = false;                   //以後この曲は諦める
        }
    }

    void StopBgm()
    {
        const int i = (int)g_current;
        g_current = BGM::MAX;
        if (0 > i || (int)BGM::MAX <= i) { return; }
        try { bgm::Stop(BGM_NAMES[i]); }                //Stop は先頭に巻き戻してから止める
        catch (...) {}
    }

    void Release()
    {
        StopBgm();
        for (auto& sound : g_se) { if (nullptr != sound) { sound->Stop(); } sound.reset(); }
        bgm::Del();             //AllStop + CoUninitialize
        g_loaded = false;
    }
}
