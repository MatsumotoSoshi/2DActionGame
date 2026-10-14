#include <Windows.h>
#include "KamataEngine.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "StageManager.h"
#include <fstream>  
#include <string>
using namespace KamataEngine;

// シーン(型)
enum class Scene { 

	kUnKnown = 0,

	kTitle,
	kGame 
};

// 現在シーン(型)
Scene scene = Scene::kUnKnown;
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;
StageManager* stageManager = nullptr;

void LoadDebugSettings() {
	// デバッグ設定ファイルのパス
	const std::string debugFilePath = "DebugSettings.ini";
	std::ifstream file(debugFilePath);

	// ファイルを開く
	if (file.is_open()) {
		std::string targetStage;
		if (std::getline(file, targetStage)) {
			// ステージを設定
			if (targetStage == "InitialStage") {
				stageManager->SetCurrentStageIndexByName(targetStage);
			}
		}
		file.close();
	}
}

void ChangeScene() {
	switch (scene) { 
	case Scene::kTitle: 
		if (titleScene->IsFinished()) {
		    // シーン変更
			scene = Scene::kGame;
			// 旧シーン開放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}

		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーン開放
			delete gameScene;
			gameScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene();
			titleScene->Initialize();
		} else if (gameScene && gameScene->IsReloadRequested()) {
			// ホットリロード: GameScene を丸ごと作り直す
			delete gameScene;
			gameScene = nullptr;

			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;

	case Scene::kGame:
		gameScene->Update();
		break;
	}

}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	
	case Scene::kGame:
		gameScene->Draw();
	    break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE2C_26_マツモト_ソウシ_2DActionGame");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();


	// ゲームシーンのインスタンス生成
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	// デバッグ設定ファイルを読み込み
	LoadDebugSettings();

	// クラスインスタンスの生成
	stageManager = new StageManager();
	stageManager->LoadStageData();

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		// シーン切り替え		
		ChangeScene();

		// 現在シーン更新
		UpdateScene();

	    // 描画開始
		dxCommon->PreDraw();

		// 現在シーン描画
		DrawScene();

		// 描画終了
		dxCommon->PostDraw();

	}

	// ゲームシーンの解放
	delete titleScene;
	delete gameScene;
	delete stageManager;

	// nullptrの代入
	titleScene = nullptr;
	gameScene = nullptr;

	// エンジンの終了処理
	Finalize();

	return 0;
}
