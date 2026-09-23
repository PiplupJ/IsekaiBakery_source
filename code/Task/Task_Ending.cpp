//-------------------------------------------------------------------
//エンディング
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Ending.h"
#include  "Task_Title.h"
#include  "GameDefine.h"
#include  "GameSound.h"
#include  "DrawInfo.h"

namespace  Ending
{
	Resource::WP  Resource::instance;

	const int SCORE_DRAW_PX = 64; //数字描画
	const int RESULT_WAIT = 90;	  //結果発表までの待ち時間
	const int SHEET_W = 1774;	  //シートの横
	const int SHEET_H = 443;	  //シートの縦
	const int BLINK_CYCLE = 60;	  //点滅週期
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->sheet = DG::Image::Create("./data/image/result.png");
		this->player = DG::Image::Create("./data/image/player_loaf_bear.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->sheet.reset();
		this->player.reset();
		return true;
	}
	//-------------------------------------------------------------------
	//「初期化」タスク生成時に１回だけ行う処理
	bool  Object::Initialize()
	{
		//スーパークラス初期化
		__super::Initialize(defGroupName, defName, true);
		//リソースクラス生成orリソース共有
		this->res = Resource::Create();

		//★データ初期化
		//★タスクの生成
		this->uiResource = UI::Resource::Create();
		this->render2D_Priority[1] = GameDefine::PRIO_ACTOR;
		this->waitTimer = RESULT_WAIT;
		this->endingState = EndingState::Wait;

		GameSound::PlayBgm(GameSound::BGM::Ending);

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放
		GameSound::StopBgm();

		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
			auto  nextTask = Title::Object::Create(true);
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		auto inp = ge->in1->GetState();

		//状態によって分岐
		switch (this->endingState)
		{
		case EndingState::Wait:
			this->waitTimer--;
			if (this->waitTimer <= 0) {
				GameSound::Play(GameSound::SE::Result);
				this->endingState = EndingState::Show;
			}
			break;
		case EndingState::Show:
			this->playerMotionFrame++;
			this->blinkCount++;
			if (inp.ST.down) {
				//自身に消滅要請
				this->Kill();
			}
			break;
		default:
			break;
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		//ロゴ描画
		this->DrawLogo();

		//結果表示はShowステート中
		if (this->endingState != EndingState::Show) { return; }

		//点数描画
		string s = to_string(ge->score);
		int scoreDigits = s.length();

		ML::Box2D scoreDraw(
			(GameDefine::SCREEN_W - scoreDigits * SCORE_DRAW_PX) / 2,
			(GameDefine::SCREEN_H - SCORE_DRAW_PX) / 2,
			SCORE_DRAW_PX,
			SCORE_DRAW_PX
		);

		UI::DrawDigits(scoreDraw, ge->score, ML::Color(1, 1, 1, 1));

		//プレイヤーキャラクター描画
		this->DrawPlayer();

		//点滅テキスト描画
		if (nullptr == ge->debugFont) { return; }
		if (BLINK_CYCLE / 2 <= this->blinkCount % BLINK_CYCLE) { return; }

		ML::Box2D  guide(0, 460, GameDefine::SCREEN_W, 32);
		ge->debugFont->Draw(guide, "PRESS  S  TO  RESTART", ML::Color(1, 1, 1, 1), DT_CENTER);
	}
	//-------------------------------------------------------------------
	//ロゴ描画
	void Object::DrawLogo()
	{
		int row = 0;
		//状態によって描画に使うシートの行を設定
		switch (this->endingState)
		{
		case EndingState::Wait:
			row = 0;
			break;
		case EndingState::Show:
			row = 1;
			break;
		default:
			break;
		}

		DrawInfo di;

		di.draw = ML::Box2D(
			(GameDefine::SCREEN_W - SHEET_W / 2) / 2,
			32,
			SHEET_W / 2,
			SHEET_H / 2
		);
		di.src = ML::Box2D(0, row * SHEET_H, SHEET_W, SHEET_H);
		di.color = ML::Color(1, 1, 1, 1);

		DrawSprite(this->res->sheet, di);
	}
	//-------------------------------------------------------------------
	//プレイヤーキャラクター描画
	void Object::DrawPlayer()
	{
		//売上が0以下なら、困っているようなモーション
		int row = ge->score > 0 ? 4 : 5;

		//アニメーション用
		int frame = (this->playerMotionFrame / GameDefine::ANIM_WAIT) % GameDefine::ANIM_FRAMES;

		DrawInfo di;
		di.draw = ML::Box2D((GameDefine::SCREEN_W-GameDefine::PLAYER_DRAW_PX) / 2, (GameDefine::SCREEN_H - SCORE_DRAW_PX) / 2 + SCORE_DRAW_PX, GameDefine::PLAYER_DRAW_PX, GameDefine::PLAYER_DRAW_PX);
		di.src = ML::Box2D(frame * GameDefine::PLAYER_PX, row * GameDefine::PLAYER_PX, GameDefine::PLAYER_PX, GameDefine::PLAYER_PX);
		di.color = ML::Color(1, 1, 1, 1);

		DrawSprite(this->res->player, di);
	}
	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//以下は基本的に変更不要なメソッド
	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//-------------------------------------------------------------------
	//タスク生成窓口
	Object::SP  Object::Create(bool  flagGameEnginePushBack_)
	{
		Object::SP  ob = Object::SP(new  Object());
		if (ob) {
			ob->me = ob;
			if (flagGameEnginePushBack_) {
				ge->PushBack(ob);//ゲームエンジンに登録
			}
			if (!ob->B_Initialize()) {
				ob->Kill();//イニシャライズに失敗したらKill
			}
			return  ob;
		}
		return nullptr;
	}
	//-------------------------------------------------------------------
	bool  Object::B_Initialize()
	{
		return  this->Initialize();
	}
	//-------------------------------------------------------------------
	Object::~Object() { this->B_Finalize(); }
	bool  Object::B_Finalize()
	{
		auto  rtv = this->Finalize();
		return  rtv;
	}
	//-------------------------------------------------------------------
	Object::Object() : playerMotionFrame(0), blinkCount(0) {	}
	//-------------------------------------------------------------------
	//リソースクラスの生成
	Resource::SP  Resource::Create()
	{
		if (auto sp = instance.lock()) {
			return sp;
		}
		else {
			sp = Resource::SP(new  Resource());
			if (sp) {
				sp->Initialize();
				instance = sp;
			}
			return sp;
		}
	}
	//-------------------------------------------------------------------
	Resource::Resource() {}
	//-------------------------------------------------------------------
	Resource::~Resource() { this->Finalize(); }
}