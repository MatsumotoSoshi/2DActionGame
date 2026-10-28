#pragma once
#include "KamataEngine.h"

class StageManager {
public:
	struct StageData {
		std::string name;   // ステージ名
		int32_t timeLimit;  // 時間制限
	};

    /// <summary>
    /// ステージデータファイルの読み込み
    /// </summary>
	void LoadStageData();

    /// <summary>
    /// ステージデータの取得
    /// </summary>
	/// <param name="index">ステージ番号</param> 
    /// <returns>ステージデータ</returns>
	const StageData& GetStageData(int32_t index) const { 
		//assert(index >= 0 && index < stageDatas_.size());
		return stageDatas_[index];
	}
	int32_t GetStageCount() const { return static_cast<int32_t>(stageDatas_.size()); }

	void SetCurrentStageIndex(int32_t index) { 
		//assert(index >= 0 && index < stageDatas_.size());
		currentStageIndex_ = index;
	}

	int32_t GetCurrentStageIndex() const { 
		return currentStageIndex_; 
	};

	/// <summary>
	/// 現在ステージのステージデータの取得
	/// </summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetCurrentStageData() const { 
		return GetStageData(currentStageIndex_);
	}

	/// <summary>
	/// ステージ名指定で現在ステージ番号設定
	/// </summary>
	/// <param name="index">ステージ番号</param>
	void SetCurrentStageIndexByName(const std::string& name);

private:
	// 全ステージデータ
	std::vector<StageData> stageDatas_;

	// 現在のステージ番号
	int32_t currentStageIndex_ = 0;

};
