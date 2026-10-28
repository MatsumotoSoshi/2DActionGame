#define NOMINMAX
#include "HitEffect.h"
#include "Operator.h"
#include <numbers>
#include <cmath>
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
	//assert(instance);
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
	for (uint32_t i = 0; i < particleWorldTransforms_.size(); ++i) {
		WorldTransform& worldTransform = particleWorldTransforms_[i];
		worldTransform.Initialize();
		worldTransform.translation_ = circleWorldTransform.translation_;
		worldTransform.scale_ = {0.08f, 0.08f, 1.0f};
		particleAngles_[i] = (2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / static_cast<float>(particleWorldTransforms_.size())) +
		                    random.getRandom(-0.3f, 0.3f);
		particleSpeeds_[i] = random.getRandom(0.95f, 1.7f);
		particleSizes_[i] = random.getRandom(0.15f, 0.3f);
		worldTransform.rotation_ = {0.0f, 0.0f, particleAngles_[i]};
	}

}

void HitEffect::Update() { 
	counter_++;

	// 状態遷移の管理
	switch (state_) {
	case State::kSpread: {
		float t = (float)counter_ / kSpreadDuration;
		
		// 中心の白い閃光を素早く膨らませる
		float scale = Easing::EaseOut(t, 0.05f, 0.42f);
		circleWorldTransform_.scale_ = {scale, scale, scale};

		// 白い破片を放射状に飛び散らせる
		for (uint32_t i = 0; i < particleWorldTransforms_.size(); ++i) {
			WorldTransform& particle = particleWorldTransforms_[i];
			float distance = Easing::EaseOut(t, 0.0f, particleSpeeds_[i]);
			float size = particleSizes_[i] * (1.0f - 0.25f * t);
			particle.translation_.x = circleWorldTransform_.translation_.x + std::cos(particleAngles_[i]) * distance;
			particle.translation_.y = circleWorldTransform_.translation_.y + std::sin(particleAngles_[i]) * distance;
			particle.scale_ = {size, size, 1.0f};
		}

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

	for (WorldTransform& worldTransform : particleWorldTransforms_) {
		worldMatrix_.Update(worldTransform);
	}
}

void HitEffect::Draw(const Camera& camera) {
	// 飛び散る破片を描画
	for (const WorldTransform& worldTransform : particleWorldTransforms_) {
		model_->Draw(worldTransform, camera, &objectColor_);
	}

	// モデルの描画（ワールド変換とカメラを渡す）
	model_->Draw(circleWorldTransform_, camera, &objectColor_);

}
