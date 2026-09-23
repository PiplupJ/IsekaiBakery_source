//-------------------------------------------------------------------
//ゲーム本編
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Game.h"
#include  "Task_Ending.h"
#include  "Task_FoodDB.h"
#include  "Task_Customer.h"
#include "Task_Controller.h"
#include  "Task_UI.h"
#include  "GameSound.h"

namespace  Game
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
		ge->score = 0;	
		this->gameState = GameState::Ready;
		this->readyTimer = GameDefine::READY_FRAME;
		this->gameTimer = GameDefine::ROUND_FRAME;
		this->closeWait = GameDefine::CLOSING_WAIT;

		//★タスクの生成
		auto db = FoodDB::Object::Create(true);
		if (nullptr == db) { return false; }
		db->SetParent(this->me.lock());
		
		auto table = Customer::Resource::Create();
		if (nullptr == table) { return false; }
		if (table->customerTable.empty()) { return false; }
		this->customerSpawner.Initialize(table->customerTable);

		auto controller = Controller::Object::Create(true);
		if (nullptr == controller) { return false; }
		controller->SetParent(this->me.lock());
		this->controller = controller;

		auto ui = UI::Object::Create(true);
		ui->SetParent(this->me.lock());
		ui->SetRemainingTime(gameTimer, GameDefine::ROUND_FRAME);
		this->ui = ui;

		GameSound::PlayBgm(GameSound::BGM::Game);
		GameSound::Play(GameSound::SE::Start);

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
			auto next = Ending::Object::Create(true);
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		//通行人生成
		if (this->gameState != GameState::Finished) {
			this->passerbySpawner.Update(this->me.lock());
		}
		
		//ゲーム状態によって分岐
		switch (this->gameState)
		{
		case GameState::Ready: //準備中
			this->readyTimer--;
			if (this->readyTimer <= 0) {
				this->gameState = GameState::Playing;
			}
			break;
		case GameState::Playing: //プレイ中
			this->customerSpawner.Update(this->me.lock());
			this->gameTimer--;
			this->ui.lock()->SetRemainingTime(gameTimer, GameDefine::ROUND_FRAME);
			//時間が切れたら、終了待機
			if (this->gameTimer <= 0) {
				this->gameTimer = 0;
				this->gameState = GameState::Closing;
			}
			break;
		case GameState::Closing: //終了待機中
		{
			auto c = this->controller.lock();
			if (c != nullptr) {
				//受けている注文があれば待つ
				if (c->IsMenuOpen()) { break; }
				//入力禁止
				c->LockControls();
			}
			this->closeWait--;
			//時間が切れたら終了
			if (this->closeWait <= 0) {
				this->gameState = GameState::Finished;
			}
		}
			break;
		case GameState::Finished:	//終了
			this->Kill();
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
	shared_ptr<Customer::Object>  Object::GetCustomerAtSeat(int slot_) const
	{
		return this->customerSpawner.CustomerAtSeat(slot_);
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