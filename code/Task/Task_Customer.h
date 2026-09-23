#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//客タスク
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "DrawInfo.h"
#include "GameDefine.h"

namespace  Customer
{
	//種類ごとのデータ　ResourceやObjectのメンバ変数として保持する
	struct CustomerData
	{
		int				id;
		DG::Image::SP 	sheet;			//画像シート(ファイルから読み込む)
		int 			sheetPixel;		//描画先の画像サイズ（px）
		int 			weight;			//出現率
		int 			patienceFrame;	//注文を待つフレーム数
		int				wantMin;		//注文する品物の最小数
		int				wantMax;		//注文する品物の最大数
		float			moveSpeed;		//歩く速度
		vector<int>		wantPool;		//注文する品物の候補
	};

	//タスクに割り当てるグループ名と固有名
	const  string  defGroupName(GameDefine::GROUP_CUSTOMER);	//グループ名
	const  string  defName("NoName");	//タスク名
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
		vector<CustomerData>	customerTable;
		DG::Image::SP			bubbleTop;
		DG::Image::SP			bubbleMid;
		DG::Image::SP			bubbleBottom;
	private:
		static 	bool  ParseRow(const string& line_, CustomerData& out_); //ファイルからデータを検出
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
		void	SetUp(int speciesId_, int slot_, const ML::Vec2& pos_);
		bool	BeginTalk();							// 待っているお客さんに話をかけるTalking -> Waiting
		void	EndTalk();								// Talking -> Waiting
		void	Receive(const vector<int>& handed_);	// Talking -> Receiving, 点数更新
		bool	IsSeated() const;						// 待っているか / Waiting || Talking || Receiving
		int		WantCount() const;						// want.size() 求める品目数

		void	RenderBubble();							// 注文バブル描画

	private:
		//客の状態
		enum class CustomerState { Walking, Entering, Waiting, Talking, Receiving, Returning };
		//注文結果判定用
		enum class ReceiveState { None, Good, Bad };

		void		ChangeState(CustomerState next_);	//状態遷移
		void		Move_Walk();	//Walkingステート用移動(横移動)
		void		Move_Enter();	//Enteringステート用移動(Y軸下向き）
		void		Move_Return();	//Returningステート用移動(Y軸上向き）
		DrawInfo	GetDrawInfo() const;	//描画用

		int				speciesId;		//種類識別用ID
		int				slot;			//待機する座席
		int				xDir;			//横移動向き
		int				patienceFrame;	//待ち時間
		int				patienceLeft;	//残り待ち時間
		int				motionFrame;	//モーション描画用カウンタ
		float			moveSpeed;		//移動速度
		ML::Vec2		pos;			//座標
		vector<int>		want;			//求める料理リスト
		CustomerState	customerState;	//状態
		ReceiveState	receiveState;	//注文結果判定
	};
}