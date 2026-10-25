#include "MapChipField.h"
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cassert>
using namespace KamataEngine;

namespace {

std::map<char, MapChipType> mapChipTypeTable= {
    {'B', MapChipType::kBlock},
    {'W', MapChipType::kGoalWall},
    {'P', MapChipType::kPlayer},
	{'E', MapChipType::kEnemy},
	{'G', MapChipType::kGoal},
};

}

void MapChipField::ResetMapChipData() { 
    mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
    for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
    }
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
    // マップチップデータをリセット
    ResetMapChipData();

    // ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

    // マップチップCSV
    std::stringstream mapChipCsv;
    // ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();
    // ファイルを閉じる
    file.close();

    // CSVからマップチップデータを読み込む
    for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

        // １行分の文字列をストリームに変換して解析しやすくする
		std::istringstream lineStream(line);

        for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			std::getline(lineStream, word, ',');

            // 空白の場合はスキップ
			if (word.empty()) {
				continue;
			}

			// 先頭文字がいずれかのマップチップ種別に該当するか確認
			if (!mapChipTypeTable.contains(word[kChipType])) {
				continue;
			}

			// 先頭文字でマップチップのタイプを判別
			mapChipData_.data[i][j].type = mapChipTypeTable[word[kChipType]];

			// サブIDを含まない場合はスキップ
			if (word.size() <= kChipSubID) {
				continue;
			}

			// サブIDを設定
			mapChipData_.data[i][j].subID = static_cast<uint8_t>(word[kChipSubID] - '0');
        }
    }
}

bool MapChipField::SaveMapChipCsv(const std::string& filePath) const {
	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}
	for (uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			if (x != 0) {
				file << ',';
			}
			const MapChipDataUnit& cell = mapChipData_.data[y][x];
			char typeCode = '0';
			switch (cell.type) {
			case MapChipType::kBlock: typeCode = 'B'; break;
			case MapChipType::kGoalWall: typeCode = 'W'; break;
			case MapChipType::kPlayer: typeCode = 'P'; break;
			case MapChipType::kEnemy: typeCode = 'E'; break;
			case MapChipType::kGoal: typeCode = 'G'; break;
			case MapChipType::kBlank: default: break;
			}
			file << typeCode;
			if (typeCode != '0') {
				file << static_cast<uint32_t>(cell.subID);
			}
		}
		file << '\n';
	}
	file.flush();
	return file.good();
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex >= kNumBlockHorizontal) {
		return MapChipType::kBlank;
	}

	if (yIndex >= kNumBlockVirtical) {
		return MapChipType::kBlank;
    }

    return mapChipData_.data[yIndex][xIndex].type;
}

void MapChipField::SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type) {
	SetMapChip(xIndex, yIndex, type, 0);
}

void MapChipField::SetMapChip(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID) {
	if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVirtical) {
		return;
	}

	mapChipData_.data[yIndex][xIndex].type = type;
	mapChipData_.data[yIndex][xIndex].subID = subID;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
    return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0);
}

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position){
    IndexSet indexSet = {};
	// 範囲外の浮動小数座標を符号なし整数へ変換すると未定義動作になるため、
	// 先にマップ外を検出して無効なインデックスを返す。
	if (position.x < -kBlockWidth / 2.0f || position.x >= kBlockWidth * (kNumBlockHorizontal - 0.5f) ||
	    position.y < -kBlockHeight / 2.0f || position.y >= kBlockHeight * (kNumBlockVirtical - 0.5f)) {
		return {kNumBlockHorizontal, kNumBlockVirtical};
	}

	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth);
	uint32_t y = static_cast<uint32_t>((position.y + kBlockHeight / 2) / kBlockHeight);
	indexSet.yIndex = kNumBlockVirtical - 1 - y;
	return indexSet;
}

// 指定ブロックの中心座標を取得する
MapChipField::Rect MapChipField::GetRectByIndex(uint32_t xIndex, uint32_t yIndex) {
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;

	return rect;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex >= kNumBlockHorizontal) {
		return 0;
	}

	if (yIndex >= kNumBlockVirtical) {
		return 0;
	}

	return mapChipData_.data[yIndex][xIndex].subID;
}
