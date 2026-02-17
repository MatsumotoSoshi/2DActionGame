#include "Player.h"
using namespace KamataEngine;

void Player::Initialize() {
	model_ = Model::CreateFromOBJ("Player");

	worldTransform_.Initialize();
	worldTransform_.translation_ = {0.0f, 0.0f, 0.0f};
	worldMatrix_.Update(worldTransform_);
}

void Player::Update() {
} 

void Player::Draw(const Camera& camera) { 
	 model_->Draw(worldTransform_, camera);
}
