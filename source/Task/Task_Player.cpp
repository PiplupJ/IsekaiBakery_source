//-------------------------------------------------------------------
// プレイヤーキャラクターのタスク 
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Player.h"

namespace  Player
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->img = DG::Image::Create("./data/image/player_loaf_bear.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->img.reset();
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
		this->pos = ML::Vec2((float)(GameDefine::SCREEN_W / 2), (float)GameDefine::PLAYER_Y);
		this->render2D_Priority[1] = GameDefine::PRIO_ACTOR - 0.2f;

		//★タスクの生成

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放


		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		this->motionFrame++;
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		DrawInfo di = this->GetDrawInfo();

		di.draw.Offset(this->pos);

		DrawSprite(this->res->img, di);
	}
	//-------------------------------------------------------------------
	//入力処理
	GameInput::InputResult  Object::HandleInput(const GameInput::Input& input)
	{
		GameInput::InputResult rtv = GameInput::InputResult::None;

		PlayerState next = input.xDir != 0 ? PlayerState::Walking : PlayerState::Idle;

		this->Move(input.xDir);
		
		//声をかけるキーを押すと、メニューを開いてみる
		if (input.selectPressed) {
			rtv = GameInput::InputResult::OpenMenu;
		}

		this->playerState = next;

		return rtv;
	}	
	//-------------------------------------------------------------------
	void Object::OnControlLost()
	{
		//待機状態にする
		this->playerState = PlayerState::Idle;
	}
	//-------------------------------------------------------------------
	void  Object::Move(int dirX_)
	{
		if (0 == dirX_) { this->xDir = 0; return; }

		this->xDir = dirX_;

		this->pos.x += dirX_ * GameDefine::PLAYER_SPEED;

		if (GameDefine::PLAYER_X_MIN > this->pos.x) { this->pos.x = (float)GameDefine::PLAYER_X_MIN; }
		if (GameDefine::PLAYER_X_MAX < this->pos.x) { this->pos.x = (float)GameDefine::PLAYER_X_MAX; }
	}
	//-------------------------------------------------------------------
	//プレイヤーが向いているスロット
	int  Object::FacingSlot() const
	{
		int i = (int)floorf((this->pos.x - GameDefine::SLOT_X0) / GameDefine::SLOT_W + 0.5f);
		if (i < 0 || GameDefine::SLOT_COUNT <= i) { return -1; }

		float slotX = (float)(GameDefine::SLOT_X0 + i * GameDefine::SLOT_W);
		if (GameDefine::SLOT_TOLERANCE < fabsf(this->pos.x - slotX)) { return -1; }

		return i;
	}
	//-------------------------------------------------------------------
	DrawInfo  Object::GetDrawInfo() const
	{

		int row = 0;

		switch (this->playerState)
		{
		case PlayerState::Idle :
			row = 3;
			break;
		case PlayerState::Walking:
			row = 1;
			break;
		default:
			break;
		}

		int frame = (this->motionFrame / GameDefine::ANIM_WAIT) % GameDefine::ANIM_FRAMES;

		DrawInfo rtv;
		rtv.draw = ML::Box2D(-GameDefine::PLAYER_DRAW_PX / 2, -GameDefine::PLAYER_DRAW_PX / 2, GameDefine::PLAYER_DRAW_PX, GameDefine::PLAYER_DRAW_PX);
		rtv.src = ML::Box2D(frame* GameDefine::PLAYER_PX, row* GameDefine::PLAYER_PX, GameDefine::PLAYER_PX, GameDefine::PLAYER_PX);
		rtv.color = ML::Color(1, 1, 1, 1);

		if (this->playerState == PlayerState::Walking && this->xDir > 0) {
			rtv.draw.x = -rtv.draw.x;
			rtv.draw.w = -rtv.draw.w;
		}

		return rtv;
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
	Object::Object() : playerState(PlayerState::Idle), motionFrame(0), xDir(0) {}
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
