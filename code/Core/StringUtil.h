//文字列読み込み用
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cmath>

namespace StringUtil {

	//文字列最初、最後の空白を消す
	inline std::string Trim(const std::string& s_)
	{
		//最初の空白を探す
		size_t begin = s_.find_first_not_of(" \t\r");
		
		//全体が空白。npos返却
		if (std::string::npos == begin) { return std::string{}; }

		//最後の空白を探す
		size_t end = s_.find_last_not_of(" \t\r");
		return s_.substr(begin, end - begin + 1);
	}

	//コンマを基準に、文字列を切り取る
	inline std::vector<std::string> SplitCsvLine(const std::string& line_)
	{
		std::vector<std::string> tokens;
		std::stringstream ss(line_);
		std::string cell;

		// コンマ区切りで各セルを読み取り、前後の空白を除去してtokensに追加
		while (std::getline(ss, cell, ',')) {
			tokens.push_back(Trim(cell));
		}

		// 空行、または行末がコンマで終わる場合は、最後に空文字列を追加
		// （getlineはコンマの後に何も無いと読み取れないため、手動で補完する）
		if (line_.empty() || line_.back() == ',') {
			tokens.push_back("");
		}

		return tokens;
	}

	//文字列をInt型に変換
	inline bool TryParseInt(const std::string& s_, int& out_)
	{
		if (s_.empty()) { return false; }
		size_t pos = 0;
		int v = 0;
		try { v = std::stoi(s_, &pos); }
		catch (const std::exception&) { return false; }

		//12abcなどの、stoiでは12として判定してしまう場合かを確認
		if (pos != s_.size()) { return false; }
		out_ = v;
		return true;
	}

	//文字列をfloat型に変換
	inline bool TryParseFloat(const std::string& s_, float& out_)
	{
		if (s_.empty()) { return false; }
		size_t pos = 0;
		float v = 0.0f;
		try { v = std::stof(s_, &pos); }
		catch (const std::exception&) { return false; }

		//12abcなどの、stofでは12として判定してしまう場合かを確認
		if (pos != s_.size()) { return false; }
		out_ = v;
		return true;
	}
	
}
