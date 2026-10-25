#define NOMINMAX
#include "Fade.h"
#include <algorithm>
using namespace KamataEngine;

void Fade::Initialize() {
	// 1つの墨の形を画面より大きく広げて、画面全体を覆う。
	uint32_t textureHandle = TextureManager::Load("particle/circle.png");
	inkSprite_ = Sprite::Create(textureHandle, {-kBlobSize * 0.5f, -kBlobSize * 0.5f}, {0.015f, 0.02f, 0.035f, 1.0f});
	inkSprite_->SetSize({0.0f, 0.0f});
}

Fade::~Fade() {
	delete inkSprite_;
}

void Fade::Start(Status status, float duration) {
status_ = status;
duration_ = duration;
counter_ = 0.0f;
	if (status_ == Status::FadeIn) {
		UpdateInk(kBlobSize, 1.0f);
	} else if (status_ == Status::FadeOut) {
		UpdateInk(0.0f, 1.0f);
	}
}

void Fade::Stop() { 
	status_ = Status::None;
}

bool Fade::IsFinished() const { 
	// フェード状態による分岐
	switch (status_) { 
	case Status::FadeIn:
	case Status::FadeOut:
		return counter_ >= duration_;
	}

	return true;
}

void Fade::Update() {
	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		// なにもしない
		break;

	case Status::FadeIn:
	{
		// フェードイン中の更新処理
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = std::min(counter_, duration_);
		
		// 1.0fから0.0fの間で、経過時間がフェード時間に近づくほどにアルファ値が小さくする
		const float alpha = duration_ > 0.0f ? std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f) : 0.0f;
		UpdateInk(kBlobSize, alpha);
		break;
	}

	case Status::FadeOut:
	{
		// フェードアウト中の更新処理
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = std::min(counter_, duration_);

		// 0.0fから1.0fの間で、経過時間がフェード時間に近づくほどにアルファ値が大きくする
		const float progress = duration_ > 0.0f ? std::clamp(counter_ / duration_, 0.0f, 1.0f) : 1.0f;
		UpdateInk(kBlobSize * progress, 1.0f);
		break;
	}
	}
}

void Fade::UpdateInk(float size, float alpha) {
	inkSprite_->SetSize({size, size});
	inkSprite_->SetPosition({640.0f - size * 0.5f, 360.0f - size * 0.5f});
	inkSprite_->SetColor({0.015f, 0.02f, 0.035f, alpha});
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	Sprite::PreDraw();
	inkSprite_->Draw();
	Sprite::PostDraw();
}
