#define NOMINMAX
#include "Fade.h"
#include <algorithm>
using namespace KamataEngine;

void Fade::Initialize() { 
uint32_t textureHandle = TextureManager::Load("white1x1.png");
sprite_ = Sprite::Create(textureHandle, {0, 0});
sprite_->SetSize(Vector2(1280, 720));
sprite_->SetColor(Vector4(0, 0, 0, 1));
}

void Fade::Start(Status status, float duration) {
status_ = status;
duration_ = duration;
counter_ = 0.0f;
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
		// フェードイン中の更新処理
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = std::min(counter_, duration_);
		
		// 1.0fから0.0fの間で、経過時間がフェード時間に近づくほどにアルファ値が小さくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));
		break;

	case Status::FadeOut:
		// フェードアウト中の更新処理
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		counter_ = std::min(counter_, duration_);

		// 0.0fから1.0fの間で、経過時間がフェード時間に近づくほどにアルファ値が大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}