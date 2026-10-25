#include "Trajectory.h"
#include <cmath>
#include <cstddef>
#include "KamataEngine.h"

void Trajectory::Initialize() {
	points_.clear();
}

void Trajectory::RecordPoint(const KamataEngine::Vector3& point) {
	if (points_.empty()) {
		points_.push_back(point);
		return;
	}

	const KamataEngine::Vector3& last = points_.back();
	const float dx = point.x - last.x;
	const float dy = point.y - last.y;
	if (dx * dx + dy * dy >= 0.0025f) {
		points_.push_back(point);
	}
}

void Trajectory::SetPoints(const std::vector<KamataEngine::Vector3>& points) {
	points_ = points;
}

void Trajectory::Draw() const {
	if (points_.size() < 2) {
		return;
	}

	const KamataEngine::Vector4 color = {0.85f, 1.0f, 0.2f, 1.0f};
	for (std::size_t i = 1; i < points_.size(); ++i) {
		KamataEngine::Vector3 start = points_[i - 1];
		KamataEngine::Vector3 end = points_[i];
		// ステージの床やブロック表面に埋もれないよう、カメラ側へ少し出す。
		start.z -= 0.55f;
		end.z -= 0.55f;
		KamataEngine::PrimitiveDrawer::GetInstance()->DrawLine3d(start, end, color);
	}

}
