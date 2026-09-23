//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_UI.h"

namespace  UI
{
	Resource::WP  Resource::instance;

	//イメージシート関係
	const int SHEET_PIXEL = 32;
	const int DIGIT_PIXEL = 32;
	const int NUMBER_ROW = 0;
	const int ICON_ROW = 1;
	const int MINUS_COL = 0;
	const int COLON_COL = 1;
	const int COIN_COL = 3;
	const int CLOCK_COL = 6;
	const int BAND_L_COL = 7;
	const int BAND_M_COL = 8;
	const int BAND_R_COL = 9;

	// 描画関係
	const int TIMER_DIGITS = 5;
	const int TIMER_COLON_INDEX = 2;
	const int TIMER_ICON_CELLS = 1;
	const int TIMER_EDGE_CELLS = 2;
	const int TIMER_MIDDLE_CELLS = TIMER_ICON_CELLS + TIMER_DIGITS;

	//時間計算関係
	const int FRAMES_PER_SECOND = 60;
	const int SECONDS_PER_MINUTE = 60;
	const int DECIMAL_BASE = 10;
	const int MAX_TIMER_MINUTES = 99;
	const int MAX_TIMER_SECONDS = (MAX_TIMER_MINUTES + 1) * SECONDS_PER_MINUTE - 1;
	const int TIMER_WARN_SEC = 10;
	const int SCORE_TWEEN_FRAME = 30;
	const int SCORE_LEFT = 8;

	const int UI_TOP = 12;
	const int UI_TIMER_X = (GameDefine::SCREEN_W - SCORE_LEFT
		- (TIMER_MIDDLE_CELLS + TIMER_EDGE_CELLS) * DIGIT_PIXEL)/2;
	const ML::Color TEXT_COLOR(1, 1, 1, 1);
	const ML::Color GOLD(1, 1, 0.9, 0.1);
	const ML::Color TIMER_FULL(1, 1, 1, 1);
	const ML::Color TIMER_WARN(1, 1, 0.1f, 0.1f);
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->sheet = DG::Image::Create("./data/image/gameUI.png");
		this->scoreBackground = DG::Image::Create("./data/image/bubble_mid.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->sheet.reset();
		this->scoreBackground.reset();
		return true;
	}
	//-------------------------------------------------------------------
	//「初期化」タスク生成時に１回だけ行う処理
	bool  Object::Initialize()
	{
		//スーパークラス初期化
		__super::Initialize(defGroupName, defName, true);
		//リソースクラス生成orリソース共有
		this->res = Resource::Create();

		//★データ初期化
		this->render2D_Priority[1] = GameDefine::PRIO_MENU;

		this->displayScore = ge->score;
		this->startScore = ge->score;
		this->targetScore = ge->score;

		this->scoreElapsed = 0;
		this->scoreUpdating = false;

		//★タスクの生成

		return  true;
	}
	//-------------------------------------------------------------------
	//「終了」タスク消滅時に１回だけ行う処理
	bool  Object::Finalize()
	{
		//★データ＆タスク解放


		if (!ge->QuitFlag() && this->nextTaskCreate) {
			//★引き継ぎタスクの生成
		}

		return  true;
	}
	//-------------------------------------------------------------------
	//「更新」１フレーム毎に行う処理
	void  Object::UpDate()
	{
		this->WatchScore();
		this->AdvanceScore();
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		this->DrawScore();
		this->DrawTimer();
	}
	//-------------------------------------------------------------------
	//残り時間更新(ゲームタスクから実行)
	void Object::SetRemainingTime(int remainingFrame, int totalFrame)
	{
		if (totalFrame <= 0) {
			this->remainingTime = 0;
			return;
		}

		this->remainingTime = remainingFrame;

		this->remainingTime = (std::max)(0, (std::min)(remainingFrame, totalFrame));
	}
	//-------------------------------------------------------------------
	void Object::WatchScore()
	{
		//更新する必要がない
		if (this->targetScore == ge->score) { return; }

		this->startScore = this->displayScore;
		this->targetScore = ge->score;
		this->scoreElapsed = 0;
		this->scoreUpdating = true;
	}
	//-------------------------------------------------------------------
	void Object::AdvanceScore()
	{
		if (!this->scoreUpdating) { return; }

		this->scoreElapsed++;

		//演出フレーム中、スコアが徐々に上昇
		this->displayScore
			= this->startScore
			+ (int)(((long long)this->targetScore - this->startScore) * this->scoreElapsed / SCORE_TWEEN_FRAME);

		//演出フレームが経ったら表示スコアを対象スコアにする	
		if (this->scoreElapsed >= SCORE_TWEEN_FRAME) {
			this->displayScore = this->targetScore;
			this->scoreUpdating = false;
		}
	}
	//-------------------------------------------------------------------
	void Object::DrawScore()
	{
		this->DrawScoreBackground();

		//コインアイコン描画
		DrawInfo icon;
		icon.draw = ML::Box2D(SCORE_LEFT, UI_TOP, DIGIT_PIXEL, DIGIT_PIXEL);
		icon.src = GetCell(COIN_COL, ICON_ROW);
		icon.color = TEXT_COLOR;
		DrawSprite(this->res->sheet, icon);

		//数字描画座標設定
		ML::Box2D drawStart = icon.draw;
		drawStart.x += DIGIT_PIXEL;

		DrawDigits(drawStart, this->displayScore, GOLD);
	}
	//-------------------------------------------------------------------
	void Object::DrawScoreBackground()
	{
		int scoreLength = to_string(ge->score).length();

		DrawInfo di;
		di.draw = ML::Box2D(SCORE_LEFT, UI_TOP, DIGIT_PIXEL * (scoreLength+1), DIGIT_PIXEL);
		di.src = ML::Box2D(0, 0, GameDefine::BUBBLE_W, GameDefine::BUBBLE_MID_H);
		di.color = ML::Color(1, 0.2, 0.2, 0.2);

		DrawSprite(this->res->scoreBackground, di);
	}
	//-------------------------------------------------------------------
	void Object::DrawTimer()
	{
		this->DrawTimerBackground();
		this->DrawTimerIcon();
		this->DrawTimerDigits();
	}
	//-------------------------------------------------------------------
	//制限時間UIの背景描画
	void Object::DrawTimerBackground()
	{
		DrawInfo di;

		//左描画
		di.draw = ML::Box2D(UI_TIMER_X, UI_TOP, DIGIT_PIXEL, DIGIT_PIXEL);
		di.src = GetCell(BAND_L_COL, ICON_ROW);
		di.color = TEXT_COLOR;
		DrawSprite(this->res->sheet, di);

		//中央は表示する数字分描画
		di.src = GetCell(BAND_M_COL, ICON_ROW);
		for (int i = 0; i < TIMER_MIDDLE_CELLS; i++) {
			di.draw.x += DIGIT_PIXEL;
			DrawSprite(this->res->sheet, di);
		}
		//右描画
		di.draw.x += DIGIT_PIXEL;
		di.src = GetCell(BAND_R_COL, ICON_ROW);
		DrawSprite(this->res->sheet, di);
	}
	//-------------------------------------------------------------------
	//制限時間の時計アイコン描画
	void Object::DrawTimerIcon()
	{
		DrawInfo di;
		di.draw = ML::Box2D(UI_TIMER_X + DIGIT_PIXEL, UI_TOP, DIGIT_PIXEL, DIGIT_PIXEL);
		di.src = GetCell(CLOCK_COL, ICON_ROW);
		di.color = TEXT_COLOR;
		DrawSprite(this->res->sheet, di);
	}
	//-------------------------------------------------------------------
	void Object::DrawTimerDigits()
	{
		//残り時間を分と秒で表現
		const int total = (std::min)(this->remainingTime / FRAMES_PER_SECOND, MAX_TIMER_SECONDS);
		const int minutes = total / SECONDS_PER_MINUTE;
		const int seconds = total % SECONDS_PER_MINUTE;

		//ex) 99:59 の形式に描画するため
		const int digits[TIMER_DIGITS] = {
			minutes / DECIMAL_BASE, minutes % DECIMAL_BASE , 0,
			seconds / DECIMAL_BASE, seconds % DECIMAL_BASE
		};

		DrawInfo di;
		di.draw = ML::Box2D(UI_TIMER_X + (1 + TIMER_ICON_CELLS) * DIGIT_PIXEL,
			UI_TOP, DIGIT_PIXEL, DIGIT_PIXEL);
		di.src = GetCell(COLON_COL, ICON_ROW);
		di.color = this->TimerColor();

		for (int i = 0; i < TIMER_DIGITS; ++i) {
			//コロンインデックスなら「：」描画
			if (i == TIMER_COLON_INDEX) { 
				DrawSprite(this->res->sheet, di); 
			}
			//数字描画
			else { 
				DrawDigits(di.draw, digits[i], di.color); 
			}
			di.draw.x += DIGIT_PIXEL;
		}
	}
	//-------------------------------------------------------------------
	ML::Color Object::TimerColor() const
	{
		const int timeLeft = this->remainingTime / FRAMES_PER_SECOND;

		if (TIMER_WARN_SEC > timeLeft) {
			return TIMER_WARN;
		}
		else {
			return TIMER_FULL;
		}
	}
	//-------------------------------------------------------------------
	//value値分の数字をstartCellから、colorの色で描画
	void DrawDigits(const ML::Box2D& startCell, int value, const ML::Color& color)
	{
		auto resource = Resource::Create();
		if (nullptr == resource) { return; }

		DrawInfo di;
		di.draw = startCell;
		di.color = color;

		for (char c : std::to_string(value)) {
			//負数なら-を描画
			const int col = ('-' == c) ? MINUS_COL : c - '0';
			const int row = ('-' == c) ? ICON_ROW : NUMBER_ROW;

			di.src = GetCell(col, row);
			DrawSprite(resource->sheet, di);
			di.draw.x += di.draw.w; //次の数字は現在の右に描画
		}
	}
	//-------------------------------------------------------------------
	ML::Box2D GetCell(int col, int row)
	{
		return ML::Box2D(col * SHEET_PIXEL, row * SHEET_PIXEL, SHEET_PIXEL, SHEET_PIXEL);
	}
	//-------------------------------------------------------------------
	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//以下は基本的に変更不要なメソッド
	//★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
	//-------------------------------------------------------------------
	//タスク生成窓口
	Object::SP  Object::Create(bool  flagGameEnginePushBack_)
	{
		Object::SP  ob = Object::SP(new  Object());
		if (ob) {
			ob->me = ob;
			if (flagGameEnginePushBack_) {
				ge->PushBack(ob);//ゲームエンジンに登録
				
			}
			if (!ob->B_Initialize()) {
				ob->Kill();//イニシャライズに失敗したらKill
			}
			return  ob;
		}
		return nullptr;
	}
	//-------------------------------------------------------------------
	bool  Object::B_Initialize()
	{
		return  this->Initialize();
	}
	//-------------------------------------------------------------------
	Object::~Object() { this->B_Finalize(); }
	bool  Object::B_Finalize()
	{
		auto  rtv = this->Finalize();
		return  rtv;
	}
	//-------------------------------------------------------------------
	Object::Object() {	}
	//-------------------------------------------------------------------
	//リソースクラスの生成
	Resource::SP  Resource::Create()
	{
		if (auto sp = instance.lock()) {
			return sp;
		}
		else {
			sp = Resource::SP(new  Resource());
			if (sp) {
				sp->Initialize();
				instance = sp;
			}
			return sp;
		}
	}
	//-------------------------------------------------------------------
	Resource::Resource() {}
	//-------------------------------------------------------------------
	Resource::~Resource() { this->Finalize(); }
}