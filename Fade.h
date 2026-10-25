#pragma once
#include "KamataEngine.h"

class Fade
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	~Fade();

	// フェードの状態
	enum class Status {
		None,         // フェードなし
		FadeIn,       // フェードイン中
		FadeOut       // フェードアウト中
	};

	// 現在のフェードの状態
	Status status_ = Status::None;

	// フェードの継続時間
	float duration_ = 0.0f;

	// 経過時間カウンター
	float counter_ = 0.0f;

	// フェード開始
	void Start(Status status, float duration);

	// フェード停止
	void Stop();

	// フェード終了判定
	bool IsFinished() const;

private:
	static constexpr float kBlobSize = 2200.0f;
	KamataEngine::Sprite* inkSprite_ = nullptr;
	void UpdateInk(float size, float alpha);

};
