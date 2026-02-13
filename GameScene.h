#pragma once
#include "KamataEngine.h"
#include "Player.h"

// ゲームシーン
class GameScene 
{
public:
	~GameScene();

	// 初期化
	void Initialize();

	// 更新
	void Update();
	
	// 描画
	void Draw();

private:
};
