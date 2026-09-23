#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//コントローラータスク
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "Task_Player.h"
#include "Task_Menu.h"
#include "GameInput.h"

namespace   Controller
{
	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName(GameDefine::GROUP_PLAYER);	//グループ名
	const  string  defName("Controller");	//タスク名
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
		void  UpDate()			override;//「実行」１フレーム毎に行う処理
		void  Render2D_AF()		override;//「2D描画」１フレーム毎に行う処理
		bool  Finalize();		//「終了」タスク消滅時に１回だけ行う処理
	//変更可◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇◇
	public:
		//追加したい変数・メソッドはここに追加する

		//public関数は、ゲームシーンで実行する
		
		//現在メニュー操作中だか
		bool IsMenuOpen() const;
		//入力ロック(演出待機用)
		void LockControls();
	private:

		//操作モード
		enum class ControlMode {
			None,	//操作なし
			Player,	//プレイヤー操作
			Menu,	//メニュー操作
			
		};
		ControlMode controlMode;

		weak_ptr<Player::Object>	player; //プレイヤーの弱参照
		weak_ptr<Menu::Object>		menu;   //メニューの弱参照
		XI::GamePad::SP				pad;	//入力装置(仮想ゲームパッド)
		bool						isLocked = false;

		//メニューを開く
		void TryOpenMenu(int slot);	
	};
}