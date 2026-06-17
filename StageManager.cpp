#include "StageManager.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
using namespace KamataEngine;

void StageManager::LoadStageData() {
	// ステージデータファイルのパス
	const std::string filePath = "Resources/fields/stageDatas.csv";

	// ifstreamでステージファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open() && "ステージデータファイルが存在しません");

	// ファイルの内容を格納するstringstreamの宣言
	std::stringstream stageDataFile;
	// ファイルの内容をstrinngstreamにコピー
	stageDataFile << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// ステージデータを最終行まで1行ずつ読み込む
	// 1行分の内容を格納するstringの宣言
	std::string line;

	while (std::getline(stageDataFile, line)) {
		// 1行分の内容を格納するstringstreamを宣言して、stringから変換
		std::istringstream lineStream(line);

		// ステージデータを格納する構造体
		StageData stageData;
		// カンマ区切りの一つ分を格納するstringの宣言
		std::string word;

		// カンマ区切りで次のデータを取得する
		std::getline(lineStream, word, ',');

		// ステージ名を格納する
		stageData.name = word;

		// カンマ区切りで次のデータを取得
		std::getline(lineStream, word, ',');

		// 整数に変換して時間制限を格納する
		stageData.timeLimit = std::stoi(word);

		// ステージテーブルデータに格納
		stageDatas_.push_back(stageData);

	}
}

void StageManager::SetCurrentStageIndexByName(const std::string& name) {
	// 全ステージデータ検索
	auto it = std::find_if(stageDatas_.begin(), stageDatas_.end(), [&name](const StageData& data) { return data.name == name; });
	    // ステージ名が一致したら現在ステージ番号を設定する
	if (it != stageDatas_.end()) {
		currentStageIndex_ = static_cast<int32_t>(std::distance(stageDatas_.begin(), it));
		// 目的を達したので関数を抜ける
		return;
	}
	assert(false && "指定されたステージ名は存在しません");
}
