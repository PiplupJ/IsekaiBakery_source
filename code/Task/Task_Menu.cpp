//-------------------------------------------------------------------
//メニュータスク
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_Menu.h"
#include  "Task_FoodDB.h"
#include  "StringUtil.h"
#include  "Task_UI.h"
#include  "GameSound.h"

namespace  Menu
{
	Resource::WP  Resource::instance;

	int Object::lastRow = 0;
	int Object::lastCol = 0;

	const int  CELL_PAD = 4;	//セルの上から料理を描く位置

	//セルの角からカーソルを描画する位置
	const int  CURSOR_X_PAD = 8;

	const int PRICE_PIXEL = 12;		//値段描画サイズ

	const int PANEL_PAD = 16;		 //背景パネルの余白
	const int PANEL_SHEET_PX = 1254; //背景パネルシートサイズ

	const int TRAY_SHEET_W = 1774;          //menu_tray.png の実寸
	const int TRAY_SHEET_H = 887;

	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		this->cursor = DG::Image::Create("./data/image/cursor.png");
		this->basketEmpty = DG::Image::Create("./data/image/basket_empty.png");
		this->menuPanel = DG::Image::Create("./data/image/menu_panel.png");
		this->basketTray = DG::Image::Create("./data/image/menu_tray.png");

		//空きマスは-1。先に全部空にしておく
		for (int r = 0; r < GameDefine::MENU_ROWS; ++r) {
			for (int c = 0; c < GameDefine::MENU_COLS; ++c) {
				this->grid[r][c] = -1;
			}
		}

		string filePath = "./data/table/Layout.csv";

		ifstream fin(filePath);
		if (!fin) { return false; }

		string line;
		bool first = true; //一行目はヘッダなので読み飛ばす
		while (getline(fin, line)) {
			if (first) { first = false; continue; }
			if (line.empty() || '#' == line[0]) { continue; }

			vector<string> tokens = StringUtil::SplitCsvLine(line);

			if (tokens.size() < 3) {
				fin.close();
				return false;
			}

			int id, row, col;

			if (!StringUtil::TryParseInt(tokens[0], id)) { return false; }
			if (!StringUtil::TryParseInt(tokens[1], row)) { return false; }
			if (!StringUtil::TryParseInt(tokens[2], col)) { return false; }
			if (row < 0 || row >= GameDefine::MENU_ROWS) { return false; }
			if (col < 0 || col >= GameDefine::MENU_COLS) { return false; }

			this->grid[row][col] = id;
		}
		fin.close();

		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->cursor.reset();
		this->basketEmpty.reset();
		this->menuPanel.reset();
		this->basketTray.reset();
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

		this->repeatWait = 0;
		this->heldDx = 0, this->heldDy = 0;
		this->cursorRow = lastRow;
		this->cursorCol = lastCol;

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
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
		auto db = FoodDB::Resource::Create();
		if (nullptr == db) { return; }

		this->RenderMenu(*db);
		this->RenderCursor();
		this->RenderBasket(*db);

	}
	//-------------------------------------------------------------------
	GameInput::InputResult Object::HandleInput(const GameInput::Input& input)
	{
		auto c = this->customer.lock();
		if (c == nullptr) { this->Close(); return GameInput::InputResult::CloseMenu; }

		//方向入力の判定。「離した」「押し始め」「押しっぱなし」三種類の判定
		//離した
		if (input.xDir == 0 && input.yDir == 0) {
			this->repeatWait = 0;
			this->heldDx = 0;
			this->heldDy = 0;
		}
		//押し始めた
		else if (input.xDir != this->heldDx || input.yDir != this->heldDy) {
			this->MoveCursor(input.yDir, input.xDir);
			this->repeatWait = GameDefine::CURSOR_DAS;
			this->heldDx = input.xDir;
			this->heldDy = input.yDir;
			GameSound::Play(GameSound::SE::Cursor);
		}
		//押しっぱなし
		else if (--this->repeatWait <= 0) {
			this->MoveCursor(input.yDir, input.xDir);
			this->repeatWait = GameDefine::CURSOR_ARR;
			GameSound::Play(GameSound::SE::Cursor);
		}

		if (input.selectPressed) {
			int id = this->res->grid[this->cursorRow][this->cursorCol];
			if ((int)this->basket.size() < this->wantCount && id >= 0) {
				this->basket.push_back(id);
				GameSound::Play(GameSound::SE::Push);
			}
		}

		if (input.backPressed) {
			if (this->basket.empty()) {
				c->EndTalk();
				GameSound::Play(GameSound::SE::Pop);
				this->Close();
				return GameInput::InputResult::CloseMenu;
			}
			this->basket.pop_back();
			GameSound::Play(GameSound::SE::Pop);
		}

		if (input.confirmPressed) {
			if ((int)this->basket.size() == this->wantCount) {
				c->Receive(this->basket);
				GameSound::Play(GameSound::SE::Hand);
				this->Close();
				return GameInput::InputResult::CloseMenu;
			}
		}

		return GameInput::InputResult::None;

	}
	//-------------------------------------------------------------------
	void Object::SetUp(const shared_ptr<Customer::Object>& c_)
	{
		if (c_ == nullptr) { return; }

		this->customer = c_;
		this->wantCount = c_->WantCount();
		this->basket.clear();
	}
	//-------------------------------------------------------------------
	void Object::MoveCursor(int rowOffset_, int colOffset_)
	{
		if (0 == rowOffset_ && 0 == colOffset_) { return; }

		int r = this->cursorRow;
		int c = this->cursorCol;
		for (;;) {
			r += rowOffset_;
			c += colOffset_;
			if (0 > r || GameDefine::MENU_ROWS <= r) { return; }
			if (0 > c || GameDefine::MENU_COLS <= c) { return; }
			if (-1 == this->res->grid[r][c]) { continue; }

			this->cursorRow = r;
			this->cursorCol = c;
			return;
		}
	}
	//-------------------------------------------------------------------
	void Object::Close()
	{
		lastRow = this->cursorRow;
		lastCol = this->cursorCol;
		this->Kill();
	}
	//-------------------------------------------------------------------
	void Object::RenderMenu(const FoodDB::Resource& db)
	{
		//背景パネル描画
		DrawInfo panel;
		panel.draw = ML::Box2D(
			GameDefine::MENU_LEFT - PANEL_PAD,
			GameDefine::MENU_TOP - PANEL_PAD,
			GameDefine::MENU_CELL * GameDefine::MENU_COLS + PANEL_PAD*2,
			GameDefine::MENU_CELL * GameDefine::MENU_ROWS + PANEL_PAD*2
		);
		panel.src = ML::Box2D(0, 0, PANEL_SHEET_PX, PANEL_SHEET_PX);
		panel.color = ML::Color(1, 1, 1, 1);
		DrawSprite(this->res->menuPanel, panel);

		//料理描画
		for (int r = 0; r < GameDefine::MENU_ROWS; ++r) {
			for (int c = 0; c < GameDefine::MENU_COLS; ++c) {
				int id = this->res->grid[r][c];
				if (-1 == id) { continue; }

				DrawInfo di;
				di.draw = ML::Box2D(
					GameDefine::MENU_LEFT + c * GameDefine::MENU_CELL + (GameDefine::MENU_CELL - GameDefine::FOOD_DRAW_PX) / 2,
					GameDefine::MENU_TOP + r * GameDefine::MENU_CELL + CELL_PAD,
					GameDefine::FOOD_DRAW_PX, GameDefine::FOOD_DRAW_PX);
				di.src = ML::Box2D(0, 0, GameDefine::FOOD_PX, GameDefine::FOOD_PX);
				di.color = ML::Color(1, 1, 1, 1);
				DrawSprite(db.Get(id).img, di);

				//値段情報描画
				ML::Box2D priceDraw;
				priceDraw = ML::Box2D(
					di.draw.x,
					di.draw.y + GameDefine::FOOD_DRAW_PX,
					PRICE_PIXEL,
					PRICE_PIXEL
				);

				UI::DrawDigits(priceDraw, db.Get(id).price, ML::Color(1, 1, 0.9, 0.1));

			}
		}
	}
	//-------------------------------------------------------------------
	void Object::RenderCursor()
	{
		DrawInfo di;
		di.draw = ML::Box2D(
			GameDefine::MENU_LEFT + this->cursorCol * GameDefine::MENU_CELL + CURSOR_X_PAD,
			GameDefine::MENU_TOP + this->cursorRow * GameDefine::MENU_CELL + CELL_PAD,
				GameDefine::FOOD_DRAW_PX, GameDefine::FOOD_DRAW_PX);
		di.src = ML::Box2D(0, 0, GameDefine::FOOD_DRAW_PX, GameDefine::FOOD_DRAW_PX);
		di.color = ML::Color(1, 1, 1, 1);
		DrawSprite(this->res->cursor, di);
	}
	//-------------------------------------------------------------------
	void Object::RenderBasket(const FoodDB::Resource& db)
	{
		DrawInfo  tray;
		tray.draw = ML::Box2D(
			GameDefine::BASKET_LEFT - PANEL_PAD*2,
			GameDefine::BASKET_TOP - PANEL_PAD,
			GameDefine::BASKET_CELL * this->wantCount + PANEL_PAD * 3,
			GameDefine::FOOD_DRAW_PX + PRICE_PIXEL + PANEL_PAD * 2);
		tray.src = ML::Box2D(0, 0, TRAY_SHEET_W, TRAY_SHEET_H);
		tray.color = ML::Color(1, 1, 1, 1);
		DrawSprite(this->res->basketTray, tray);

		int total = 0;
		for (int i = 0; i < this->wantCount; ++i) {
			DrawInfo di;
			di.draw = ML::Box2D(
				GameDefine::BASKET_LEFT + i * GameDefine::BASKET_CELL, GameDefine::BASKET_TOP,
				GameDefine::FOOD_DRAW_PX, GameDefine::FOOD_DRAW_PX);
			di.color = ML::Color(1, 1, 1, 1);

			if (i < (int)this->basket.size()) {
				di.src = ML::Box2D(0, 0, GameDefine::FOOD_PX, GameDefine::FOOD_PX);
				DrawSprite(db.Get(this->basket[i]).img, di);
				total += db.Get(this->basket[i]).price;
			}
			else {
				di.src = ML::Box2D(0, 0, GameDefine::FOOD_DRAW_PX, GameDefine::FOOD_DRAW_PX);
				DrawSprite(this->res->basketEmpty, di);
			}
		}
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