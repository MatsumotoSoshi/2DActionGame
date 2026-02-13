#include "Player.h"
using namespace KamataEngine;

void Player::Initialize(uint32_t textureHandle) {
	textureHandle_ = textureHandle;

	model_ = Model::Create();

	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
	worldTransform_.TransferMatrix();
}

void Player::Update() {

}

void Player::Draw(const Camera& camera) { 
	model_->Draw(worldTransform_, camera, textureHandle_);
}
