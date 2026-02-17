#pragma once
#include "KamataEngine.h"
#include "WorldMatrix.h"

class Skydome 
{
public:
	/// <summary>
	/// 
	/// </summary>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	/// <summary>
	/// 
	/// </summary>
	void Update();

	/// <summary>
	/// 
	/// </summary>
	void Draw();

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Camera* camera_ = nullptr;
	KamataEngine::Model* model_ = nullptr;

	WorldMatrix worldMatrix_;
};
