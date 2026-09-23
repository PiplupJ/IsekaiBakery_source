#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//客生成用
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "Task_Customer.h"

class CustomerSpawner
{
private:
	vector<int>					bag;			//生成待機リスト
	size_t						bagIndex;		//現在生成するインデックス
	weak_ptr<Customer::Object>	seats[GameDefine::SLOT_COUNT];	//各座席の客
	int							spawnTimer;		//生成タイマー
	int							elapsedFrame;	//経過時間

	int 	NextCustomer();			//次の客生成
	int     GetFreeSlot() const;	//空いている座席

public:
	CustomerSpawner();

	void Initialize(const vector<Customer::CustomerData>& table_);	//初期化
	void Update(const BTask::SP parent_);							//更新
	shared_ptr<Customer::Object> CustomerAtSeat(int slot_) const;	//座席の客情報
};