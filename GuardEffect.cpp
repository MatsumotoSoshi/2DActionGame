#include "GuardEffect.h"
#include "Operator.h"
#include <numbers>
#include "Random.h"
#include "Easing.h"
using namespace KamataEngine;

// 静的メンバ変数の実体
KamataEngine::Model* GuardEffect::model_ = nullptr;
KamataEngine::Camera* GuardEffect::camera_ = nullptr;

GuardEffect* GuardEffect::Create(const WorldTransform& circleWorldTransform) {
	// インスタンス生成
	GuardEffect* instance = new GuardEffect();
	// newの失敗を検出
	assert(instance);
	// インスタンスの初期化
	instance->Initialize(circleWorldTransform);
	// 初期化したインスタンスを返す
	return instance;
}

void GuardEffect::Initialize(const WorldTransform& circleWorldTransform) { 
	circleWorldTransform_.Initialize();
	Vector3 offset = {-0.5f, 0.0f, 0.0f}; 

	this->circleWorldTransform_.translation_ = circleWorldTransform.translation_ + offset;

	objectColor_.Initialize();
	color_ = {0.9f, 0, 0, 1};

}

void GuardEffect::Update() { 
	counter_++;

	// 状態遷移の管理
	switch (state_) {
	case State::kSpread: {
		float t = (float)counter_ / kSpreadDuration;

		// スプレッド中はスケーリングをイージング
		float scale = Easing::EaseOut(t, 0.1f, 2.5f);
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

}

void GuardEffect::Draw(const Camera& camera) {
	// モデルの描画（ワールド変換とカメラを渡す）
	model_->Draw(circleWorldTransform_, camera, &objectColor_);

}