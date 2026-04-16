#include "HitEffect.h"
#include "Operator.h"
#include <numbers>
#include "Random.h"
#include "Easing.h"
using namespace KamataEngine;

// 静的メンバ変数の実体
KamataEngine::Model* HitEffect::model_ = nullptr;
KamataEngine::Camera* HitEffect::camera_ = nullptr;

HitEffect* HitEffect::Create(const WorldTransform& circleWorldTransform) {
	// インスタンス生成
	HitEffect* instance = new HitEffect();
	// newの失敗を検出
	assert(instance);
	// インスタンスの初期化
	instance->Initialize(circleWorldTransform);
	// 初期化したインスタンスを返す
	return instance;
}

void HitEffect::Initialize(const WorldTransform& circleWorldTransform) { 
	circleWorldTransform_.Initialize();
	this->circleWorldTransform_.translation_ = circleWorldTransform.translation_;

	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};

	Random random;
	float radius = 1.5f;
	for (WorldTransform& worldTransform : ellipseWorldTransform_) {
		worldTransform.scale_ = {0.1f, 1.7f, 1.0f};

		float angle = random.getRotation(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);

		worldTransform.rotation_ = {0.0f, 0.0f, angle};
		worldTransform.translation_ = circleWorldTransform.translation_;

		worldTransform.Initialize();
	}

}

void HitEffect::Update() { 
	counter_++;

	// 状態遷移の管理
	switch (state_) {
	case State::kSpread: {
		float t = (float)counter_ / kSpreadDuration;

		// スプレッド中はスケーリングをイージング
		float scale = Easing::EaseOut(t, 0.1f, 1.0f);
		circleWorldTransform_.scale_ = {scale, scale, scale};

		if (counter_ >= kSpreadDuration) {
			state_ = State::kFade;
			counter_ = 0;
		}
		break;
	}

	case State::kFade: {
		float t = (float)counter_ / kFadeDuration;

		// フェード中はアルファ値をイージング
		color_.w = Easing::EaseIn(t, 1.0f, 0.0f);

		if (counter_ >= kFadeDuration) {
			state_ = State::kDeath;
		}
		break;
	}

	case State::kDeath:
		break;
	}

	objectColor_.SetColor(color_);

	worldMatrix_.Update(circleWorldTransform_);

	for (WorldTransform& worldTransform : ellipseWorldTransform_) {
		worldMatrix_.Update(worldTransform);
	}
}

void HitEffect::Draw(const Camera& camera) {
	// 楕円を描画
	for (const WorldTransform& worldTransform : ellipseWorldTransform_) {
		model_->Draw(worldTransform, camera, &objectColor_);
	}

	// モデルの描画（ワールド変換とカメラを渡す）
	model_->Draw(circleWorldTransform_, camera, &objectColor_);

}