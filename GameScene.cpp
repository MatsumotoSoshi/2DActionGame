#include "GameScene.h"
using namespace KamataEngine;

GameScene::~GameScene() { 
}

void GameScene::Initialize() {
}

void GameScene::Update() {
	player_->Update();
}

void GameScene::Draw() { 
	player_->Draw(); 
}


