//-------------------------------------------------------------------
//客生成用
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "CustomerSpawner.h"

//-------------------------------------------------------------------
CustomerSpawner::CustomerSpawner()
	: bagIndex(0)
	, spawnTimer(GameDefine::SPAWN_START)
	, elapsedFrame(0)
{
	for (auto& seat : seats) {
		seat.reset();
	}
}
//-------------------------------------------------------------------
void CustomerSpawner::Initialize(const vector<Customer::CustomerData>& table_)
{
	//生成待機リスト作成
	bag.clear();
	for (size_t i = 0; i < table_.size(); ++i) {
		for (int n = 0; n < table_[i].weight; ++n) {
			bag.push_back(table_[i].id);
		}
	}

	//後ろから 1 つずつ、それより前のどれかと入れ替える。全順列が等確率になる
	for (int i = (int)bag.size() - 1; 0 < i; --i) {
		swap(bag[i], bag[rand() % (i + 1)]);
	}

	bagIndex = 0;
	spawnTimer = GameDefine::SPAWN_START;
	elapsedFrame = 0;
}
//-------------------------------------------------------------------
void CustomerSpawner::Update(const BTask::SP parent_)
{
	++elapsedFrame;
	--spawnTimer;
	//生成時間ではない
	if (spawnTimer > 0) {
		return;
	}

	int slot = this->GetFreeSlot();
	//空席がない
	if (slot < 0) {
		return; 
	}
	//次の客データ生成
	int  speciesId = this->NextCustomer();

	//生成座標設定。画面外の左か右か
	ML::Vec2 spawnPos;

	if (rand() % 2 == 0) {
		spawnPos = GameDefine::SPAWN_POS_L;
	}
	else {
		spawnPos = GameDefine::SPAWN_POS_R;
	}

	//客タスク生成
	auto customer = Customer::Object::Create(true);
	if (customer == nullptr) {
		return;
	}

	//客タスクをゲームタスクの子の設定
	customer->SetParent(parent_);
	customer->SetUp(speciesId, slot, spawnPos); //初期化
	this->seats[slot] = customer;	//座席に客をセッティング

	//ゲーム進行度を確認
	const float progress = (std::max)(0.0f, (std::min)(1.0f,
		static_cast<float>(elapsedFrame) / GameDefine::ROUND_FRAME));
	
	//次の客生成までの時間を計算
	this->spawnTimer =static_cast<int>(GameDefine::SPAWN_START
		- progress * (GameDefine::SPAWN_START - GameDefine::SPAWN_END));
	if (this->spawnTimer < GameDefine::SPAWN_END) {
		this->spawnTimer = GameDefine::SPAWN_END;
	}
}
//-------------------------------------------------------------------
//座席の客返却
shared_ptr<Customer::Object> CustomerSpawner::CustomerAtSeat(int slot_) const
{
	//座席が正しい数値か
	if (slot_ < 0 || GameDefine::SLOT_COUNT <= slot_) {
		return nullptr;
	}

	//客がいるか
	auto customer = this->seats[slot_].lock();
	if (customer == nullptr) {
		return nullptr;
	}
	if (!customer->IsSeated()) {
		return nullptr;
	}

	return customer;
}
//-------------------------------------------------------------------
int CustomerSpawner::NextCustomer() 
{
	if (this->bag.empty()) {
		return 0;
	}
	if (this->bagIndex >= this->bag.size()) {
	
	//後ろから 1 つずつ、それより前のどれかと入れ替える。全順列が等確率になる
	for (int i = (int)bag.size() - 1; 0 < i; --i) {
		swap(bag[i], bag[rand() % (i + 1)]);
	}

		this->bagIndex = 0;
	}

	return bag[this->bagIndex++];
}
//-------------------------------------------------------------------
int CustomerSpawner::GetFreeSlot() const
{
	for (int i = 0; i < GameDefine::SLOT_COUNT; ++i) {
		auto customer = this->seats[i].lock();
		if (customer == nullptr || !customer->IsSeated()) {
			return i;
		}
	}
	//空席がない
	return -1; 
}