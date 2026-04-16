#pragma once
#include <random>
class Random {
public:
	
	// メルセンヌ・の初期化
	Random() {
		// メルセンヌ・ツイスターエンジンの初期化
		randomEngine.seed(seedGenerator());
	}

	// 指定範囲の乱数生成器（浮動小数点数用）
	float getRotation(float min, float max) {
		// 乱数範囲ごとの初期化
		std::uniform_real_distribution<float> rotationDistribution(min, max);

		// 乱数の取得
		float result = rotationDistribution(randomEngine);
		return result;
	}

private:
	// 乱数生成エンジン
	std::random_device seedGenerator;
	// メルセンヌ・ツイスターエンジン(64bit版)
	std::mt19937_64 randomEngine;
};
