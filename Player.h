#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"

class MapChipField;

class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 移動
	/// </summary>
	void Move();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);

	KamataEngine::Vector3 velocity_ = {};
	static inline const float kAcceleration = 0.05f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.8f;

	enum class LRDirection { kRight, kLeft };

	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 旋回時間
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度
	static inline const float kGravityAcceleration_ = 0.05f;
	// 最大落下速度
	static inline const float kLimitFallSpeed_ = 2.0f;
	// ジャンプ初速
	static inline const float kJumpAcceleration_ = 0.65f;

	bool landing_ = false;

	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceilingCollision = false;   // 天井衝突
		bool landing = false;            // 着地
		bool wallContact = false;        // 壁接触
		KamataEngine::Vector3 amountOfMovement;       // 移動量
	};

	// マップ衝突判定
	void CollisionMap(CollisionMapInfo& info);

	// 角
	enum Corner {
		kRightBottom,       // 右下
		kLeftBottom,        // 左下
		kRightTop,          // 右上
		kLeftTop,           // 左上

		kNumCorner          // 要素数
	};

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	static inline const float kBlank = 0.01f;

	// 判定結果を反映して移動させる
	void ResultMove(const CollisionMapInfo& info);
	 
	// 天井に接触している場合の処理
	void CollisionCeiling(const CollisionMapInfo& info);

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform worldTransform_;
	WorldMatrix worldMatrix_;
	MapChipField* mapChipField_ = nullptr;
};
