//-------------------------------------------------------------------
//コントローラータスク
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Controller.h"
#include  "Task_Game.h"
#include  "Task_Customer.h"

namespace  Controller
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
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
		this->pad = ge->in1;
		this->controlMode = ControlMode::Player;

		//★タスクの生成

		//プレイヤー生成
		auto player = Player::Object::Create(true);
		if (nullptr == player) { return false; }
		//自身の子タスクにする
		player->SetParent(this->me.lock());
		this->player = player;

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
		//入力ロック中は実行しない
		if (this->isLocked) {
			return;
		}
		
		//仮想ゲームパッドの状態確認
		auto inp = this->pad->GetState();

		//入力を変数として定義
		GameInput::Input input;

		if (inp.LStick.BL.on) {
			input.xDir = -1;
		}
		else if (inp.LStick.BR.on) {
			input.xDir = 1;
		}
		if (inp.LStick.BU.on) {
			input.yDir = -1;
		}
		else if (inp.LStick.BD.on) {
			input.yDir = 1;
		}

		input.selectPressed = inp.B1.down;
		input.backPressed = inp.B2.down;
		input.confirmPressed = inp.B3.down;

		//操作対象に入力を送る
		switch (this->controlMode)
		{
		case ControlMode::Player:
			if (auto p = this->player.lock()) {
				//処理結果がメニュー実行だったら操作モードを切り替える
				if (p->HandleInput(input) == GameInput::InputResult::OpenMenu) {
					this->TryOpenMenu(p->FacingSlot());
				}
			}
			break;
		case ControlMode::Menu:
			if (auto m = this->menu.lock()) {
				//処理結果がメニューモード終了だったら、操作モードを切り替える
				if (m->HandleInput(input) == GameInput::InputResult::CloseMenu) {
					this->controlMode = ControlMode::Player;
				}
			}
			//メニューがなくなったら操作モードを切り替える
			else {
				this->controlMode = ControlMode::Player;
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
	}
	//-------------------------------------------------------------------
	//現在メニュー操作中だか
	bool Object::IsMenuOpen() const
	{
		return this->controlMode == ControlMode::Menu || !this->menu.expired();
	}
	//-------------------------------------------------------------------
	//入力ロック(演出待機用)
	void Object::LockControls()
	{
		this->isLocked = true;
		if (auto p = this->player.lock()) { p->OnControlLost(); }
	}
	//-------------------------------------------------------------------
	//メニューを開く
	void Object::TryOpenMenu(int slot)
	{
		//引数が正しくない
		if (slot < 0 || slot >= GameDefine::SLOT_COUNT) { return; }

		auto game = ge->GetTask<Game::Object>(
			Game::defGroupName, Game::defName);
		if (!game) { return; }

		//ゲームシーンから現在注文を受けている客を確認
		auto c = game->GetCustomerAtSeat(slot);
		if (!c) { return; }

		//注文受付
		if (!c->BeginTalk()) { return; }

		//メニュー生成
		auto m = Menu::Object::Create(true);
		if (!m) {
			c->EndTalk(); 
			return;
		}

		//メニューを自身の子タスクにして、初期化
		m->SetParent(this->me.lock());
		m->SetUp(c);
		this->menu = m;

		//メニュー操作モードに切り替える
		this->controlMode = ControlMode::Menu;
		if (auto p = this->player.lock()) {
			p->OnControlLost();
		}
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
	Object::Object() {	}
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