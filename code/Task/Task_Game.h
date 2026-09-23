#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//ゲーム本編
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "CustomerSpawner.h"
#include "PasserbySpawner.h"

namespace Controller { class Object;  }
namespace UI { class Object; }

namespace  Game
{
	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName(GameDefine::GROUP_SCENE);	//グループ名
	const  string  defName("Game");	//タスク名
	//-------------------------------------------------------------------
	class  Resource : public BResource
	{
		bool  Initialize()	override;
		bool  Finalize()	override;
		Resource();
	public:
		~Resource();
		typedef  shared_ptr<Resource>	SP;
		typedef  weak_ptr<Resource>		WP;
		static   WP  instance;
		static  Resource::SP  Create();
		//共有する変数はここに追加する
	};
	//-------------------------------------------------------------------
	class  Object : public  BTask
	{
		//変更不可◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆◆
	public:
		virtual  ~Object();
		typedef  shared_ptr<Object>		SP;
		typedef  weak_ptr<Object>		WP;
		//生成窓口 引数はtrueでタスクシステムへ自動登録
		static  Object::SP  Create(bool flagGameEnginePushBack_);
		Resource::SP	res;
	private:
		Object();
		bool  B_Initialize();
		bool  B_Finalize();
		bool  Initialize();	//「初期化」タスク生成時に１回だけ行う処理
		void  UpDate()			override;	//「実行」１フレーム毎に行う処理
		void  Render2D_AF()		override;	//「2D描画」１フレーム毎に行う処理
		bool  Finalize();	//「終了」タスク消滅時に１回だけ行う処理
		//変更可◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇
	public:
		//追加したい変数・メソッドはここに追加する
		shared_ptr<Customer::Object> GetCustomerAtSeat(int slot_) const;

	private:
		enum class GameState { Ready, Playing, Closing, Finished };
		
		GameState		gameState;				//ゲーム状態
		int				readyTimer;				//開始までの待ち時間
		int				gameTimer;				//ゲーム終了までの時間
		int				closeWait;				//ゲーム終了後、エンディングシーン遷移までの時間
		CustomerSpawner customerSpawner;		//客生成
		PasserbySpawner passerbySpawner;		//通行人生成
		weak_ptr<Controller::Object> controller;//コントローラー弱参照
		weak_ptr<UI::Object> ui;				//ui弱参照
	};
}