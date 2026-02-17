#include "WorldMatrix.h"
using namespace KamataEngine;

void WorldMatrix::Update(WorldTransform& worldTransform) {
	worldTransform.matWorld_ = MathUtility::MakeTranslateMatrix(worldTransform.translation_);
	
	worldTransform.TransferMatrix();
}