#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//  プレイヤーキャラクターのタスク 
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "DrawInfo.h"
#include "GameInput.h"

namespace  Player
{
	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName(GameDefine::GROUP_PLAYER);	//グループ名
	const  string  defName("Player");	//タスク名
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
		DG::Image::SP img;
	};
	//-------------------------------------------------------------------
	class  Object : public  BTask, public GameInput::IInputReceiver
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

		//プレイヤーが向いている方向のスロット番号を返す
		int			FacingSlot() const;

		GameInput::InputResult HandleInput(const GameInput::Input& input) override;
		void OnControlLost(); //コントローラーの操作対象ではない

	private:
		enum class PlayerState{ Idle, Walking};
		PlayerState playerState;
		DrawInfo	GetDrawInfo() const;
		ML::Vec2	pos;
		int			xDir;
		int			motionFrame;
		// dirX_ * PLAYER_SPEED分移動。カウンタのスロットに収まるように制限する
		void		Move(int dirX_);
	};
}
