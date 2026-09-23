#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
// メニュータスク
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "DrawInfo.h"
#include "Task_Customer.h"
#include "GameInput.h"

//前方宣言
namespace FoodDB {
	class Resource;
}

namespace  Menu
{
	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName(GameDefine::GROUP_UI);	//グループ名
	const  string  defName("Menu");	//タスク名
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
		DG::Image::SP	cursor;
		DG::Image::SP	basketEmpty;
		DG::Image::SP	menuPanel;
		DG::Image::SP	basketTray;
		int				grid[GameDefine::MENU_ROWS][GameDefine::MENU_COLS];	// food id, -1 where empty
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
		
		//注文を受けている客情報を基に設定
		void	SetUp(const shared_ptr<Customer::Object>& c_);
		//入力処理
		GameInput::InputResult HandleInput(const GameInput::Input& input) override;
	private:
		//カーソル移動。行(上下)と列(左右)の順で受け取る
		void	MoveCursor(int rowOffset_, int colOffset_);
		//メニューを閉じる
		void	Close();

		void RenderMenu(const FoodDB::Resource& db);	//メニュー描画
		void RenderCursor();							//カーソル描画
		void RenderBasket(const FoodDB::Resource& db);	//渡す料理描画

		weak_ptr<Customer::Object>	customer;				//客の弱参照
		int							wantCount;				//客が注文した料理数
		vector<int>					basket;					//客に渡す料理リスト
		int							cursorRow, cursorCol;	//カーソル座標
		int							repeatWait;				//次のカーソル移動までの残りフレーム
		int							heldDx, heldDy;			//前フレームの方向入力。押し始めか押しっぱなしかの判別用
		static int					lastRow, lastCol;		//直前実行時のカーソル座標
	};
}
