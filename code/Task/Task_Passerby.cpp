//-------------------------------------------------------------------
//通行人タスク
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Passerby.h"

namespace  Passerby
{
	Resource::WP  Resource::instance;

	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->sheet = DG::Image::Create("./data/image/passerby.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->sheet.reset();
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
		this->render2D_Priority[1] = GameDefine::PRIO_STREET;
		//★タスクの生成
		//種類はランダムで決定
		this->type = static_cast<PasserbyType>(rand()%PasserbyType::Count);

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
		this->pos.x += this->xDir * this->movespeed;
		++this->motionFrame;

		//画面外に出たら消える
		if (this->xDir > 0) {
			if (GameDefine::SCREEN_W + GameDefine::PASSERBY_MARGIN < this->pos.x) {
				this->Kill();
			}
		}
		else {
			if (-GameDefine::PASSERBY_MARGIN > this->pos.x) {
				this->Kill();
			}
		}
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		DrawInfo di = this->GetDrawInfo();

		di.draw.Offset(this->pos);

		DrawSprite(this->res->sheet, di);
	}
	//-------------------------------------------------------------------
	DrawInfo Object::GetDrawInfo() const
	{
		int row = 0;

		//種類によって描画する行が変える
		switch (this->type)
		{
			case PasserbyType::VillagerM: row = 0; break;
			case PasserbyType::VillagerF: row = 1; break;
			default: break;
		}

		//アニメーション用
		int frame = (this->motionFrame / GameDefine::PASSERBY_ANIM_WAIT) % GameDefine::PASSERBY_ANIM_FRAMES;

		DrawInfo rtv;

		rtv.src = ML::Box2D(frame * GameDefine::PASSERBY_PX, row * GameDefine::PASSERBY_PX, GameDefine::PASSERBY_PX, GameDefine::PASSERBY_PX);
		rtv.draw = ML::Box2D(-GameDefine::PASSERBY_DRAW_PX / 2, -GameDefine::PASSERBY_DRAW_PX / 2, GameDefine::PASSERBY_DRAW_PX, GameDefine::PASSERBY_DRAW_PX);
		rtv.color = ML::Color(0.9, 1, 1, 1); //エクストラキャラクターのため、すこし透明

		//移動向きによって左右反転
		if (this->xDir > 0) {
			rtv.draw.x = -rtv.draw.x;
			rtv.draw.w = -rtv.draw.w;
		}

		return rtv;
	}
	//-------------------------------------------------------------------
	//初期化
	void Object::SetUp(int xDir, int lane, float movespeed)
	{
		this->xDir = xDir;
		this->movespeed = movespeed;
		this->motionFrame = 0;

		float startX;
		if (xDir > 0) {
			startX = -(float)GameDefine::PASSERBY_MARGIN;
		}
		else {
			startX = (float)(GameDefine::SCREEN_W + GameDefine::PASSERBY_MARGIN);
		}
		this->pos = ML::Vec2(startX, (float)lane);
	}
	//-------------------------------------------------------------------
	float Object::GetSpeed() const
	{
		return this->movespeed;
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