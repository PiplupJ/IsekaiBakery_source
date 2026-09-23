//-------------------------------------------------------------------
// 街を歩く通行人生成。ゲームタスクが実行
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "PasserbySpawner.h"

//-------------------------------------------------------------------
PasserbySpawner::PasserbySpawner()
	:spawnWait(0)
{
	this->spawnWait = this->NextWait();
}
//-------------------------------------------------------------------
void  PasserbySpawner::Update(const BTask::SP& parent_)
{
	--this->spawnWait;
	if (0 < this->spawnWait) { return; }
	this->spawnWait = this->NextWait();

	int lane = rand() % LANE_COUNT;
	int dirX = (0 == lane) ? 1 : -1;
	int laneY = (0 == lane) ? GameDefine::PASSERBY_LANE_FAR : GameDefine::PASSERBY_LANE_NEAR;

	int steps = GameDefine::PASSERBY_SPEED_MAX_T - GameDefine::PASSERBY_SPEED_MIN_T + 1;
	float speed = (GameDefine::PASSERBY_SPEED_MIN_T + rand() % steps) * GameDefine::PASSERBY_SPEED_UNIT;

	//同じレーンの前の人より速くならないようにする。
	auto lead = this->lastInLane[lane].lock();
	if (nullptr != lead && lead->GetSpeed() < speed) { speed = lead->GetSpeed(); }

	auto o = Passerby::Object::Create(true);
	if (nullptr == o) { return; }
	o->SetParent(parent_);
	o->SetUp(dirX, laneY, speed);
	this->lastInLane[lane] = o;
}
//-------------------------------------------------------------------
int  PasserbySpawner::NextWait() const
{
	int range = GameDefine::PASSERBY_SPAWN_MAX - GameDefine::PASSERBY_SPAWN_MIN + 1;
	return GameDefine::PASSERBY_SPAWN_MIN + rand() % range;
}
