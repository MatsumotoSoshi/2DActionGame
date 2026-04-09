#include "DeathParticles.h"
#include "Operator.h"
#include <algorithm>
using namespace KamataEngine;

void DeathParticles::Initialize(Model* model, Camera* camera, Vector3& position) {
	model_ = model;
	camera_ = camera;

	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};

	// ワールド変換の初期化
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}
}

void DeathParticles::Update() {
	if (isFinished_) {
		return;
	}

	// カウンターを1フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = {kSpeed, 0, 0};
		// 回転角を計算する
		float angle = kAngleUnit * i;
		// Ｚ軸周り回転行列
		Matrix4x4 matrixRotation = MathUtility::MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = MathUtility::Transform(velocity, matrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	color_.w = std::clamp(1.0f - (counter_ / kDuration), 0.0f, 1.0f);
	// 色変換オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);
		
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldMatrix_.Update(worldTransform);
	}
}

void DeathParticles::Draw(const Camera& camera) {
	if (isFinished_) {
		return;
	}

   for (auto& worldTransform : worldTransforms_) {
       model_->Draw(worldTransform, camera, &objectColor_);
   }
}