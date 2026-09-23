//-------------------------------------------------------------------
// 料理データベース。 Resourceクラスに保持する
//-------------------------------------------------------------------
#include  "MyPG.h"
#include  "Task_FoodDB.h"
#include  "StringUtil.h"

namespace  FoodDB
{
	Resource::WP  Resource::instance;
	//-------------------------------------------------------------------
	//リソースの初期化
	bool  Resource::Initialize()
	{
		//仮データから読み込む
		this->placeholder.id = -1;
		this->placeholder.img = DG::Image::Create("./data/image/Food/placeHolder.png");
		this->placeholder.price = 0;

		string filePath = "./data/table/FoodTable.csv";

		ifstream fin(filePath);
		if (!fin) { return false; }

		string line;
		bool first = true;
		while (getline(fin, line)) {
			if (first) { first = false; continue; }		//一行目はヘッダなので読み飛ばす
			if (line.empty() || '#' == line[0]) { continue; }  //空白の行、コメントは飛ばす

			//正しく作成されたデータはテーブルに追加
			Food data;
			if (ParseRow(line, data)) {
				this->foodTable.push_back(data);
			}
			else {
				fin.close();
				return false;
			}
		}
		fin.close();

		return true;
	}
	//-------------------------------------------------------------------
	//リソースの解放
	bool  Resource::Finalize()
	{
		this->foodTable.clear();
		this->placeholder.img.reset();
		return true;
	}
	//-------------------------------------------------------------------
	//CSVファイルの行から料理データを検出
	bool  Resource::ParseRow(const string& line_, Food& out_)
	{
		vector<string> tokens = StringUtil::SplitCsvLine(line_);

		//データは最小3個
		if (tokens.size() < 3) {
			return false; 
		}
		//数字データを検出
		if (!StringUtil::TryParseInt(tokens[0], out_.id)) { return false; }
		if (!StringUtil::TryParseInt(tokens[2], out_.price)) { return false; }
		//値段が負数
		if (out_.price < 0) { return false; }

		//ファイル経路からイメージ生成
		if (auto img = DG::Image::Create("./data/image/Food/" + tokens[1])) {
			out_.img = img;
		}
		//ないなら失敗
		else {
			return false;
		}

		return true;
	}
	//-------------------------------------------------------------------
	//IDに相当する料理返却
	const Food& Resource::Get(int id_) const
	{
		//DBに存在しない料理の場合
		if (id_ < 0 || (int)this->foodTable.size() <= id_) { return this->placeholder; }

		return this->foodTable[id_];
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
		// this task only keeps the Resource alive for the round
	}
	//-------------------------------------------------------------------
	//「２Ｄ描画」１フレーム毎に行う処理
	void  Object::Render2D_AF()
	{
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
	Object::Object() {}
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
