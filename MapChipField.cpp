#include "MapChipField.h"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
using namespace KamataEngine;

namespace {

std::map<char, MapChipType> mapChipTypeTable = {
    {'B', MapChipType::kBlock   },
    {'W', MapChipType::kGoalWall},
    {'P', MapChipType::kPlayer  },
    {'E', MapChipType::kEnemy   },
    {'G', MapChipType::kGoal    },
};

}

void MapChipField::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(numBlockVirtical_);
	for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(numBlockHorizontal_);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file) {
		throw std::runtime_error("Cannot open stage CSV: " + filePath);
	}

	// 読み込みに失敗した場合は、以前のマップを変更しない。
	std::vector<std::vector<MapChipDataUnit>> rows;
	size_t width = 0;
	std::string line;
	while (std::getline(file, line)) {
		// UTF-8 BOM と CRLF に対応する。
		if (rows.empty() && line.compare(0, 3, "\xEF\xBB\xBF") == 0) {
			line.erase(0, 3);
		}
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		std::vector<MapChipDataUnit> row;
		std::istringstream lineStream(line);
		std::string word;
		while (std::getline(lineStream, word, ',')) {
			MapChipDataUnit cell{};
			const size_t first = word.find_first_not_of(" \t");
			if (first != std::string::npos) {
				word = word.substr(first, word.find_last_not_of(" \t") - first + 1);
				const auto it = mapChipTypeTable.find(word[0]);
				if (it != mapChipTypeTable.end()) {
					cell.type = it->second;
					if (word.size() > 1) {
						unsigned int id = 0;
						const char* end = word.data() + word.size();
						const auto result = std::from_chars(word.data() + 1, end, id);
						if (result.ec != std::errc{} || result.ptr != end || id > 255) {
							throw std::runtime_error("Invalid subID: " + filePath + " line " + std::to_string(rows.size() + 1));
						}
						cell.subID = static_cast<uint8_t>(id);
					}
				}
			}
			row.push_back(cell);
		}
		// 空行と行末の空セルもマップの一部として保持する。
		if (line.empty() || line.back() == ',') {
			row.push_back(MapChipDataUnit{});
		}
		width = max(width, row.size());
		rows.push_back(std::move(row));
	}

	if (file.bad() || rows.empty() || width == 0 || rows.size() > (std::numeric_limits<uint32_t>::max)() || width > (std::numeric_limits<uint32_t>::max)()) {
		throw std::runtime_error("Invalid stage CSV: " + filePath);
	}

	// 列数の少ない行は空白で埋め、すべての行の幅を揃える。
	for (auto& row : rows) {
		row.resize(width, MapChipDataUnit{});
	}

	numBlockVirtical_ = static_cast<uint32_t>(rows.size());
	numBlockHorizontal_ = static_cast<uint32_t>(width);
	mapChipData_.data = std::move(rows);
}

bool MapChipField::SaveMapChipCsv(const std::string& filePath) const {
	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}
	for (uint32_t y = 0; y < numBlockVirtical_; ++y) {
		for (uint32_t x = 0; x < numBlockHorizontal_; ++x) {
			if (x != 0) {
				file << ',';
			}
			const MapChipDataUnit& cell = mapChipData_.data[y][x];
			char typeCode = '0';
			switch (cell.type) {
			case MapChipType::kBlock:
				typeCode = 'B';
				break;
			case MapChipType::kGoalWall:
				typeCode = 'W';
				break;
			case MapChipType::kPlayer:
				typeCode = 'P';
				break;
			case MapChipType::kEnemy:
				typeCode = 'E';
				break;
			case MapChipType::kGoal:
				typeCode = 'G';
				break;
			case MapChipType::kBlank:
			default:
				break;
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
	if (xIndex >= numBlockHorizontal_) {
		return MapChipType::kBlank;
	}

	if (yIndex >= numBlockVirtical_) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex].type;
}

void MapChipField::SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type) { SetMapChip(xIndex, yIndex, type, 0); }

void MapChipField::SetMapChip(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID) {
	if (xIndex >= numBlockHorizontal_ || yIndex >= numBlockVirtical_) {
		return;
	}

	mapChipData_.data[yIndex][xIndex].type = type;
	mapChipData_.data[yIndex][xIndex].subID = subID;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (numBlockVirtical_ - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	// 範囲外の浮動小数座標を符号なし整数へ変換すると未定義動作になるため、
	// 先にマップ外を検出して無効なインデックスを返す。
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || numBlockHorizontal_ == 0 || numBlockVirtical_ == 0 || position.x < -kBlockWidth / 2.0f ||
	    position.x >= kBlockWidth * (numBlockHorizontal_ - 0.5f) || position.y < -kBlockHeight / 2.0f || position.y >= kBlockHeight * (numBlockVirtical_ - 0.5f)) {
		return {numBlockHorizontal_, numBlockVirtical_};
	}

	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth);
	uint32_t y = static_cast<uint32_t>((position.y + kBlockHeight / 2) / kBlockHeight);
	indexSet.yIndex = numBlockVirtical_ - 1 - y;
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
	if (xIndex >= numBlockHorizontal_) {
		return 0;
	}

	if (yIndex >= numBlockVirtical_) {
		return 0;
	}

	return mapChipData_.data[yIndex][xIndex].subID;
}