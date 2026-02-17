#include "Skydome.h"
using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera) {
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = { 0.0f, 0.0f, 0.0f };
}

void Skydome::Update() {
	worldTransform_.translation_ = camera_->translation_;

	worldMatrix_.Update(worldTransform_);
}

void Skydome::Draw() {
	model_->Draw(worldTransform_, *camera_);
}
