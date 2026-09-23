//-------------------------------------------------------------------
//客タスク
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Customer.h"
#include  "Task_FoodDB.h"
#include  "StringUtil.h"
#include  "GameSound.h"

namespace  Customer
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		string filePath = "./data/table/CustomerTable.csv";

		ifstream fin(filePath);
		if (!fin) { return false; }

		string line;
		bool first = true; //一行目はヘッダなので読み飛ばす
		while (getline(fin, line)) {
			if (first) { first = false; continue; }
			if (line.empty() || '#' == line[0]) { continue; } //空白の行、コメントは飛ばす

			//正しく作成されたデータはテーブルに追加
			CustomerData data;
			if (ParseRow(line, data)) {
				this->customerTable.push_back(data);
			}
			//データに間違いがあれば失敗
			else {
				fin.close();
				return false;
			}
		}
		fin.close();

		this->bubbleTop = DG::Image::Create("./data/image/bubble_top.png");
		this->bubbleMid = DG::Image::Create("./data/image/bubble_mid.png");
		this->bubbleBottom = DG::Image::Create("./data/image/bubble_bottom.png");
		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->customerTable.clear();
		this->bubbleTop.reset();
		this->bubbleMid.reset();
		this->bubbleBottom.reset();

		return true;
	}
	//-------------------------------------------------------------------
	//CSVファイルの行から客データを検出
	bool  Resource::ParseRow(const string& line_, CustomerData& out_)
	{
		vector<string> tokens = StringUtil::SplitCsvLine(line_);

		if (tokens.size() < 9) {
			return false; //データは最小9個
		}

		//数字データを検出
		if (!StringUtil::TryParseInt(tokens[0], out_.id)) { return false; }
		if (!StringUtil::TryParseInt(tokens[2], out_.sheetPixel)) { return false; }
		if (!StringUtil::TryParseInt(tokens[3], out_.weight)) { return false; }
		if (!StringUtil::TryParseInt(tokens[4], out_.patienceFrame)) { return false; }
		if (!StringUtil::TryParseInt(tokens[5], out_.wantMin)) { return false; }
		if (!StringUtil::TryParseInt(tokens[6], out_.wantMax)) { return false; }
		if (!StringUtil::TryParseFloat(tokens[7], out_.moveSpeed)) { return false; }

		//注文範囲確認 最小注文数が最大値より大きいか、最小値が1未満かを確認
		if (out_.wantMin > out_.wantMax || out_.wantMin < 1) { return false; }

		auto db = FoodDB::Resource::instance.lock();
		if (!db) { return false; }

		for (size_t i = 8; i < tokens.size(); ++i) {
			if (tokens[i].empty()) break;
			int  wantId;
			if (!StringUtil::TryParseInt(tokens[i], wantId)) {
				return false;
			}
			//DB上で存在している料理を求めているかを判定
			if (wantId < 0 ||
				static_cast<size_t>(wantId) >= db->foodTable.size()) {
				return false;
			}

			out_.wantPool.push_back(wantId);
		}
		//注文リストがないなら失敗
		if (out_.wantPool.empty()) { return false; }

		//ファイル経路からイメージ生成
		if (auto sheet = DG::Image::Create("./data/image/Customer/" + tokens[1])) {
			out_.sheet = sheet;
		}
		//ないなら失敗
		else {
			return false;
		}

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
		if (this->res->customerTable.empty()) { return false; }

		this->render2D_Priority[1] = GameDefine::PRIO_STREET;

		//★データ初期化

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
		//状態によって分岐
		switch (this->customerState)
		{
		case CustomerState::Walking: //ストリートで移動中
			this->Move_Walk();
			break;
		case CustomerState::Entering: //店へ移動中
			this->Move_Enter();
			break;
		case CustomerState::Waiting: //注文待機中
			//残り待機時間が0になったら帰る
			this->patienceLeft--;
			if (this->patienceLeft <= 0) {
				this->ChangeState(CustomerState::Returning);
			}
			break;
		case CustomerState::Talking: //料理渡し中
			break;
		case CustomerState::Receiving: //料理を渡された
			if (GameDefine::REACT_FRAME <= this->motionFrame) {
				this->ChangeState(CustomerState::Returning);
			}
			break;
		case CustomerState::Returning: //帰り
			this->Move_Return();
			break;
		default:
			break;
		}

		++this->motionFrame;
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		DrawInfo di = this->GetDrawInfo();

		di.draw.Offset(this->pos);

		DrawSprite(this->res->customerTable[this->speciesId].sheet, di);
		
		//注文リスト描画は、待機または
		if (this->customerState == CustomerState::Waiting || this->customerState == CustomerState::Talking) {
			this->RenderBubble();
		}
	}
	//-------------------------------------------------------------------
	//初期化
	void Object::SetUp(int speciesId_, int slot_, const ML::Vec2& pos_)
	{
		this->speciesId = speciesId_;
		this->slot = slot_;
		this->pos = pos_;

		const CustomerData& data_ = this->res->customerTable[speciesId_];

		this->moveSpeed = data_.moveSpeed;
		this->patienceFrame = data_.patienceFrame;
		this->patienceLeft = data_.patienceFrame;

		int count = data_.wantMin + rand() % (data_.wantMax - data_.wantMin + 1);
		this->want.clear();
		for (int i = 0; i < count; ++i) {
			this->want.push_back(data_.wantPool[rand() % data_.wantPool.size()]);   //同じ料理の複数注文許容
		}

		//出現位置、予約した座席を基に移動方向設定
		float slotX = (float)(GameDefine::SLOT_X0 + slot_ * GameDefine::SLOT_W);
		this->xDir = (slotX > pos_.x) ? 1 : -1;

		this->receiveState = ReceiveState::None;

		this->ChangeState(CustomerState::Walking);
	}
	//-------------------------------------------------------------------
	//状態遷移
	void Object::ChangeState(CustomerState next_)
	{
		//状態が変えたら、モーションフレームを初期化
		if (this->customerState != next_) {
			this->motionFrame = 0;
			this->customerState = next_;
		}
		//ストリート移動中ではないと、描画優先度をACTORに設定
		if (next_ != CustomerState::Walking) {
			this->render2D_Priority[1] = GameDefine::PRIO_ACTOR;
		}
	}
	//-------------------------------------------------------------------
	//ストリート移動中
	void Object::Move_Walk()
	{
		this->pos.x += this->xDir * this->moveSpeed;

		//目標座席(スロット)まで移動
		float slotX = (float)(GameDefine::SLOT_X0 + this->slot * GameDefine::SLOT_W);
		if (fabsf(this->pos.x - slotX) <= this->moveSpeed) {
			this->pos.x = slotX;
			this->ChangeState(CustomerState::Entering);
		}
	}
	//-------------------------------------------------------------------
	//店に入場
	void Object::Move_Enter()
	{
		this->pos.y += this->moveSpeed;

		//目標座席(スロット)まで移動
		float targetY = (float)GameDefine::COUNTER_Y - 32;
		if (fabsf(this->pos.y - targetY) <= this->moveSpeed) {
			this->pos.y = targetY;
			this->ChangeState(CustomerState::Waiting);
		}
	}
	//-------------------------------------------------------------------
	//注文確認(プレイヤーが声をかけた時実行)
	bool Object::BeginTalk()
	{
		if (CustomerState::Waiting != this->customerState) { return false; }
		this->ChangeState(CustomerState::Talking);
		return true;
	}
	//-------------------------------------------------------------------
	//注文終了
	void Object::EndTalk()
	{
		this->ChangeState(CustomerState::Waiting);
	}
	//-------------------------------------------------------------------
	//料理を渡す(プレイヤーの方で実行）
	void Object::Receive(const vector<int>& handed_)
	{
		//渡された料理と注文リストを比べる
		vector<int> w = this->want;
		vector<int> h = handed_;
		sort(w.begin(), w.end());
		sort(h.begin(), h.end());

		const bool isCorrect = (w == h);
		//正解だったら、Good
		if (isCorrect) {
			this->receiveState = ReceiveState::Good;
			GameSound::Play(GameSound::SE::Good);
		}
		//間違いがあったらBad
		else {
			this->receiveState = ReceiveState::Bad;
			GameSound::Play(GameSound::SE::Bad);
		}

		//DBから値段情報を検出し、スコア更新
		auto db = FoodDB::Resource::Create();
		size_t wi = 0, hi = 0;
		while (wi < w.size() && hi < h.size()) {
			if (w[wi] == h[hi]) { 
				ge->score += db->Get(w[wi]).price; ++wi; ++hi; 
			}
			//注文したが、もらわなかった
			else if (w[wi] < h[hi]) { 
				++wi; 
			}
			//もらったが、注文しなかった
			else { 
				ge->score -= (int)(db->Get(h[hi]).price * GameDefine::LOSS_RATE); ++hi; 
			} 
		}

		//渡した料理の中、残ったのが間違った分。スコアにペナルティ適用
		for (; hi < h.size(); ++hi) { ge->score -= (int)(db->Get(h[hi]).price * GameDefine::LOSS_RATE); }

		this->ChangeState(CustomerState::Receiving);
	}
	//-------------------------------------------------------------------
	//帰り中の移動
	void Object::Move_Return()
	{
		this->pos.y -= this->moveSpeed;

		//帰り演出が終わったら消える
		if (GameDefine::LEAVE_FADE_FRAME <= this->motionFrame) {
			this->Kill();
		}
	}
	//-------------------------------------------------------------------
	//注文リスト描画。吹き出しの様子
	void Object::RenderBubble()
	{
		if (CustomerState::Waiting != this->customerState && CustomerState::Talking != this->customerState) { return; }

		int n = (int)this->want.size();
		//残り待機時間分、吹き出しが透明になる。
		float alpha = (std::max)(0.3f, (float)this->patienceLeft / this->patienceFrame);
		ML::Color color(alpha, 1, 1, 1);
		
		int px = GameDefine::CUSTOMER_DRAW_PX;
		auto db = FoodDB::Resource::Create();

		//吹き出しの上段描画
		ML::Box2D top(-GameDefine::BUBBLE_W / 2, 
					  -px / 2 - GameDefine::BUBBLE_BOTTOM_H - n * GameDefine::BUBBLE_MID_H - GameDefine::BUBBLE_TOP_H, 
					  GameDefine::BUBBLE_W, 
					  GameDefine::BUBBLE_TOP_H);

		this->res->bubbleTop->Draw(top.OffsetCopy(this->pos), ML::Box2D(0, 0, GameDefine::BUBBLE_W, GameDefine::BUBBLE_TOP_H), color);

		//吹き出しの中段。注文リストの分描画
		for (int i = 0; i < n; ++i) {

			int midX = -GameDefine::BUBBLE_W / 2;
			int midY = -px / 2 - GameDefine::BUBBLE_BOTTOM_H - (n - i) * GameDefine::BUBBLE_MID_H;

			ML::Box2D mid(midX, midY, GameDefine::BUBBLE_W, GameDefine::BUBBLE_MID_H);

			this->res->bubbleMid->Draw(mid.OffsetCopy(this->pos), ML::Box2D(0, 0, GameDefine::BUBBLE_W, GameDefine::BUBBLE_MID_H), color);

			ML::Box2D food(midX + (GameDefine::BUBBLE_W - GameDefine::FOOD_DRAW_PX) / 2, 
						   midY + (GameDefine::BUBBLE_MID_H - GameDefine::FOOD_DRAW_PX) / 2, 
						   GameDefine::FOOD_DRAW_PX, 
						   GameDefine::FOOD_DRAW_PX);

			//吹き出しに注文リストの料理描画
			db->Get(this->want[i]).img->Draw(food.OffsetCopy(this->pos), ML::Box2D(0, 0, GameDefine::FOOD_PX, GameDefine::FOOD_PX), color);
		}

		//吹き出しの下描画
		ML::Box2D bottom(-GameDefine::BUBBLE_W / 2, 
						 -px / 2 - GameDefine::BUBBLE_BOTTOM_H, 
						 GameDefine::BUBBLE_W, 
						 GameDefine::BUBBLE_BOTTOM_H);

		this->res->bubbleBottom->Draw(bottom.OffsetCopy(this->pos), ML::Box2D(0, 0, GameDefine::BUBBLE_W, GameDefine::BUBBLE_BOTTOM_H), color);
	}
	//-------------------------------------------------------------------
	//描画情報
	DrawInfo Object::GetDrawInfo() const
	{
		int px = this->res->customerTable[this->speciesId].sheetPixel;

		int row = 0;
		//状態によってシートで描画する行設定
		switch (this->customerState)
		{
		case CustomerState::Walking:   row = 1; break;
		case CustomerState::Entering:  row = 0; break;
		case CustomerState::Returning: row = 2; break;
		case CustomerState::Receiving: row = (this->receiveState==ReceiveState::Good) ? 4 : 5; break;
		default:                       row = 3; break;
		}
		//モーションフレームを基に、描画する画像設定
		int frame = (this->motionFrame / GameDefine::ANIM_WAIT) % GameDefine::ANIM_FRAMES;

		int draw = GameDefine::CUSTOMER_DRAW_PX;

		DrawInfo rtv;
		rtv.src = ML::Box2D(frame * px, row * px, px, px);
		rtv.draw = ML::Box2D(-draw / 2, -draw / 2, draw, draw);
		rtv.color = ML::Color(1, 1, 1, 1);

		//帰る時は、時間の経過によって透明になる
		if (CustomerState::Returning == this->customerState) {
			float alpha = 1.0f - (float)this->motionFrame / GameDefine::LEAVE_FADE_FRAME;
			if (alpha < 0.0f) { alpha = 0.0f; }
			rtv.color = ML::Color(alpha, 1, 1, 1);
		}

		//横移動中、向きによって左右反転
		if (CustomerState::Walking == this->customerState && this->xDir > 0) {
			rtv.draw.x = -rtv.draw.x;
			rtv.draw.w = -rtv.draw.w;
		}

		return rtv;
	}
	//-------------------------------------------------------------------
	//同じ座席に複数の客が配置されないため
	bool Object::IsSeated() const
	{
		return this->customerState != CustomerState::Returning;
	}
	//-------------------------------------------------------------------
	//注文数返却
	int Object::WantCount() const
	{
		return (int)this->want.size();
	}

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
	Object::Object()
		:speciesId(-1), slot(0), xDir(0), patienceFrame(0), patienceLeft(0), motionFrame(0), moveSpeed(0.0f), customerState(CustomerState::Walking), receiveState(ReceiveState::None)
	{
	}
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
				if (!sp->Initialize()) { return nullptr; }
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
