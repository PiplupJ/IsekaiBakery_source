#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
// 街を歩く通行人生成。ゲームタスクが実行
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"
#include "GameDefine.h"
#include "Task_Passerby.h"

class PasserbySpawner
{
	static const int			LANE_COUNT = 2;

	weak_ptr<Passerby::Object>	lastInLane[LANE_COUNT];
	int							spawnWait;

	int   NextWait() const;
public:
	PasserbySpawner();
	void  Update(const BTask::SP& parent_);
};
