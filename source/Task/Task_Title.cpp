//-------------------------------------------------------------------
//タイトル画面
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Title.h"
#include  "Task_Game.h"
#include  "Task_Background.h"
#include  "GameSound.h"

namespace  Title
{
	Resource::WP  Resource::instance;

	const int LOGO_W = 1536;
	const int LOGO_H = 1024;
	const int LOGO_DRAW_W = LOGO_W / 2;           
	const int LOGO_DRAW_H = LOGO_H / 2;
	const int BLINK_CYCLE = 60;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->logo = DG::Image::Create("./data/image/logo.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->logo.reset();
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
		this->titleState = TitleState::Ready;
		this->logoPosY = -GameDefine::SCREEN_H / 2;
		//★タスクの生成

		//背景の重複生成を防ぐ
		auto bg = ge->GetTask<Background::Object>(Background::defGroupName, Background::defName);
		if (!bg) {
			bg = Background::Object::Create(true);
		}

		GameSound::PlayBgm(GameSound::BGM::Title);

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
			auto  nextTask = Game::Object::Create(true);
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		auto inp = ge->in1->GetState();

		//状態によって分岐
		switch (this->titleState) 
		{
		case TitleState::Ready:
			this->logoPosY+=4;
			//ロゴが中央に着いたら、開始可能になる
			if (this->logoPosY >= (GameDefine::SCREEN_H - LOGO_DRAW_H) / 2) {
				this->logoPosY = (GameDefine::SCREEN_H - LOGO_DRAW_H)/2;
				this->titleState = TitleState::CanStart;
			}
			break;
		case TitleState::CanStart:
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
		ML::Box2D drawLogo((GameDefine::SCREEN_W - LOGO_DRAW_W) / 2, this->logoPosY, LOGO_DRAW_W, LOGO_DRAW_H);
		ML::Box2D srcLogo(0, 0, LOGO_W, LOGO_H);

		this->res->logo->Draw(drawLogo, srcLogo);

		if (this->titleState!=TitleState::CanStart) { return; }
		//点滅テキスト描画
		if (nullptr == ge->debugFont) { return; }
		if (BLINK_CYCLE / 2 <= this->blinkCount % BLINK_CYCLE) { return; }

		ML::Box2D  guide(0, 460 , GameDefine::SCREEN_W, 32);
		ge->debugFont->Draw(guide, "PRESS  S  TO  START", ML::Color(1, 1, 1, 1), DT_CENTER);
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
	Object::Object() : logoPosY(0), blinkCount(0) {	}
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