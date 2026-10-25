#pragma once
#include "KamataEngine.h"
#include <vector>

class Trajectory {
public:
	void Initialize();
	void RecordPoint(const KamataEngine::Vector3& point);
	void SetPoints(const std::vector<KamataEngine::Vector3>& points);
	const std::vector<KamataEngine::Vector3>& GetPoints() const { return points_; }
	void Draw() const;

private:
	std::vector<KamataEngine::Vector3> points_;
};
