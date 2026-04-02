#pragma once
#include <cstdint>
#include <vector>
#include <KamataEngine.h>
#include <math/Vector3.h>

enum class MapChipType { 
	kBlank,
	kBlock
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField
{
public:

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

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

	

private:
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;
};
