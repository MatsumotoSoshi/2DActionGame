#pragma once
#include "KamataEngine.h"

class WorldMatrix
{
public:
	static void Update(KamataEngine::WorldTransform& worldTransform);

private:
	 static KamataEngine::Matrix4x4 MakeAffineMatrix(
		 const KamataEngine::Vector3& scale,
		 const KamataEngine::Vector3& rotation,
		 const KamataEngine::Vector3& translation
	 );
};
