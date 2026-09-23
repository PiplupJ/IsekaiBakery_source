#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
// ゲーム上で使う数値のまとめ
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"

namespace GameDefine
{
	//タスクグループ
	const string	GROUP_SCENE("Scene"); //シーン
	const string	GROUP_CUSTOMER("Customer"); //お客さん
	const string	GROUP_PLAYER("Player"); //プレイヤー関係
	const string	GROUP_UI("UI"); //UI
	const string	GROUP_STREET("Street"); //背景, 背景キャラクター


	//描画優先度
	const float		PRIO_STREET = 0.7f;		//背景キャラクター
	const float		PRIO_ACTOR = 0.5f;		//キャラクター
	const float		PRIO_MENU = 0.2f;		//メニュー、UI

	//画面サイズ
	const int		SCREEN_W = 960;
	const int		SCREEN_H = 540;
	const int		SPLIT_Y = 340; //画面の下部

	//お客さん生成関係
	const int		SLOT_COUNT = 8;
	const int		SLOT_X0 = 130;
	const int		SLOT_W = 100;
	const int		COUNTER_Y = 260;		//注文を受けるカウンタ
	//画面外からお客さん生成
	const ML::Vec2	SPAWN_POS_L = ML::Vec2(-40.0f, 168.0f);
	const ML::Vec2	SPAWN_POS_R = ML::Vec2(1000.0f, 168.0f);

	//ゲームタイマー関係
	const int       READY_FRAME = 1* 60; //ラウンド開始までの待機時間
	const int		ROUND_FRAME = 120 * 60; //ラウンド制限時間
	const int		CLOSING_WAIT = 90; //ラウンド終了以降シーン転換までの待機時間

	//お客さん描画サイズ
	const int		CUSTOMER_DRAW_PX = 64;

	//お客さんの行動関係
	const int		REACT_FRAME = 64;		//渡したアイテムに反応するフレーム
	const int		LEAVE_FADE_FRAME = 80;		//店から離れ、見えなくなるフレーム
	const int		ANIM_WAIT = 8;		//描画フレーム転換にかかるフレーム
	const int		ANIM_FRAMES = 8;		//描画フレーム数

	//注文リスト描画用
	const int		BUBBLE_W = 40;
	const int		BUBBLE_TOP_H = 6;
	const int		BUBBLE_MID_H = 36;
	const int		BUBBLE_BOTTOM_H = 10;

	//アイテム(飲食)描画用
	const int		FOOD_PX = 16;		//シート間隔
	const int		FOOD_DRAW_PX = 32;		//描画サイズ

	//メニューカーソルの連続移動
	//押した瞬間に1マス動く。押し続けると CURSOR_DAS 待ってから、以降は CURSOR_ARR 間隔で動く
	const int		CURSOR_DAS = 12; //最初の繰り返しまでの待ちフレーム。1マスだけ動かしたいのに2マス進むのを防ぐ
	const int		CURSOR_ARR = 4; //2回目以降の繰り返し間隔(フレーム)。遠いマスまで速く行くため

	//点数基準
	const float		LOSS_RATE = 0.30f;	//間違えた料理の値段に掛け算した分のペナルティ

	//プレイヤー関係
	const int		PLAYER_PX = 256;		//シート間隔
	const int		PLAYER_DRAW_PX = 64;		//描画サイズ
	const int		PLAYER_SPEED = 4;	//移動速度
	const int		PLAYER_Y = 300;		// 初期配置
	const int		PLAYER_X_MIN = SLOT_X0; // 最小X座標
	const int		PLAYER_X_MAX = SLOT_X0 + (SLOT_COUNT - 1) * SLOT_W; // 最大X座標
	const int		SLOT_TOLERANCE = 40;		//スロットの許容範囲

	//キャラクター生成関係
	const int		SPAWN_START = 180;		//生成週期の初期値
	const int		SPAWN_END = 60;		//最終直

	const int		PASSERBY_PX = 256;	//シート間隔
	const int		PASSERBY_DRAW_PX = 64;	//描画サイズ

	//通行人生成用
	const int		PASSERBY_LANE_FAR = 104;	//右向きの行
	const int		PASSERBY_LANE_NEAR = 168;	//左向きの行
	const int		PASSERBY_MARGIN = 48;	//画面外に出たと判定する基準
	const int		PASSERBY_SPAWN_MIN = 30;	//最小生成週期
	const int		PASSERBY_SPAWN_MAX = 90;	//最大生成週期
	const int		PASSERBY_SPEED_MIN_T = 10;	//最小速度
	const int		PASSERBY_SPEED_MAX_T = 22;	//最大速度
	const float		PASSERBY_SPEED_UNIT = 0.1f;	//１フレームに進む単位
	const int		PASSERBY_ANIM_WAIT = 6;	//モーションセル転換にかかるフレーム
	const int		PASSERBY_ANIM_FRAMES = 8;	//モーションセル数

	//メニューグリッド
	const int		MENU_ROWS = 5;
	const int		MENU_COLS = 5;
	const int		MENU_LEFT = 40;
	const int		MENU_TOP = SPLIT_Y - 60;
	const int		MENU_CELL = 48;		//セルサイズ。内部に料理描画
	const int		BASKET_LEFT = MENU_LEFT + (MENU_COLS + 1) * MENU_CELL;
	const int		BASKET_TOP = SPLIT_Y + 40;
	const int		BASKET_CELL = 48;
	
}