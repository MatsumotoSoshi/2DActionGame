#pragma once
#include <cstdint>
#include <vector>
#include <KamataEngine.h>
#include <math/Vector3.h>

enum class MapChipType { 
	kBlank,  // 空白
	kBlock,  // ブロック
	kGoalWall, // 敵全滅で開くゴール前の壁
	kPlayer, // プレイヤー
	kEnemy,  // 敵
	kGoal,   // ゴール
};

inline bool IsSolidMapChipType(MapChipType type) {
	return type == MapChipType::kBlock || type == MapChipType::kGoalWall;
}

struct MapChipDataUnit {
	MapChipType type;  // マップチップの種類
	uint8_t subID;     // サブID
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

enum MapChipCharIndex {
	kChipType = 0,   // マップチップタイプ
	kChipSubID = 1,  // タイプごとのサブID
};

class MapChipField
{
public:

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);
	bool SaveMapChipCsv(const std::string& filePath) const;

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type);
	void SetMapChip(uint32_t xIndex, uint32_t yIndex, MapChipType type, uint8_t subID = 0);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() const { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	struct Rect {
		float left;      // 左端
		float right;     // 右端
		float bottom;    // 下端
		float top;	     // 上端
	};

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);

private:
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;
};
