#pragma once
#include <KamataEngine.h>
#include <cstdint>
#include <math/Vector3.h>
#include <string>
#include <vector>

enum class MapChipType {
	kBlank,    // 空白
	kBlock,    // ブロック
	kGoalWall, // 敵全滅で開くゴール前の壁
	kPlayer,   // プレイヤー
	kEnemy,    // 敵
	kGoal,     // ゴール
};

inline bool IsSolidMapChipType(MapChipType type) { return type == MapChipType::kBlock || type == MapChipType::kGoalWall; }

struct MapChipDataUnit {
	MapChipType type = MapChipType::kBlank; // マップチップの種類
	uint8_t subID = 0;                      // サブID
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

enum MapChipCharIndex {
	kChipType = 0,  // マップチップタイプ
	kChipSubID = 1, // タイプごとのサブID
};

class MapChipField {
public:
	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);
	bool SaveMapChipCsv(const std::string& filePath) const;

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type);
	void SetMapChip(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID = 0);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() const { return numBlockVirtical_; }
	uint32_t GetNumBlockHorizontal() const { return numBlockHorizontal_; }

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	struct Rect {
		float left;   // 左端
		float right;  // 右端
		float bottom; // 下端
		float top;    // 上端
	};

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);

private:
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	uint32_t numBlockVirtical_ = 0;
	uint32_t numBlockHorizontal_ = 0;

	MapChipData mapChipData_;
};