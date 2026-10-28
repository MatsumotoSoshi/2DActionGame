#include <Windows.h>
#include "KamataEngine.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "ClearScene.h"
#include "StageManager.h"
#include <fstream>  
#include <string>
#include <vector>
#include "StageSelectScene.h"
#include <exception>
#include "GameAudio.h"
using namespace KamataEngine;

// シーン(型)
enum class Scene {
	kUnKnown = 0,
	kTitle,
	kStageSelect,
	kGame,
	kClear
};

// 現在シーン(型)
Scene scene = Scene::kUnKnown;
TitleScene* titleScene = nullptr;
StageSelectScene* stageSelectScene = nullptr;
GameScene* gameScene = nullptr;
ClearScene* clearScene = nullptr;
StageManager* stageManager = nullptr;

void LoadDebugSettings() {
	// デバッグ設定ファイルのパス
	const std::string debugFilePath = "DebugSettings.ini";
	std::ifstream file(debugFilePath);

	// ファイルを開く
	if (file.is_open()) {
		std::string targetStage;
		if (std::getline(file, targetStage)) {
			for (int32_t i = 0; i < stageManager->GetStageCount(); ++i) {
				if (stageManager->GetStageData(i).name == targetStage) {
					stageManager->SetCurrentStageIndex(i);
					break;
				}
			}
		}
		file.close();
	}
}

void ChangeScene() {
	switch (scene) {

	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// タイトルからステージ選択へ
			delete titleScene;
			titleScene = nullptr;

			scene = Scene::kStageSelect;
			stageSelectScene = new StageSelectScene();
			stageSelectScene->Initialize(stageManager);
		}
		break;

	case Scene::kStageSelect:
		if (stageSelectScene->IsFinished()) {
			// 選択したステージでゲーム開始
			delete stageSelectScene;
			stageSelectScene = nullptr;

			scene = Scene::kGame;
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			const bool cleared = gameScene->IsCleared();

			const uint32_t clearWallHits = gameScene->GetWallHitCount();

			const float clearDistance = gameScene->GetTotalMoveDistance();

			std::vector<Vector3> clearPath;
			if (cleared) {
				clearPath = gameScene->GetTrajectory().GetPoints();
			}

			delete gameScene;
			gameScene = nullptr;

			if (cleared) {
				scene = Scene::kClear;
				clearScene = new ClearScene();
				clearScene->Initialize(clearPath, stageManager, clearWallHits, clearDistance);
			} else {
				// ゲーム失敗時はタイトルへ戻る
				scene = Scene::kTitle;
				titleScene = new TitleScene();
				titleScene->Initialize();
			}

		} else if (gameScene->IsReloadRequested()) {
			// ホットリロード
			delete gameScene;
			gameScene = nullptr;

			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			ClearScene::ReturnTarget target = clearScene->GetReturnTarget();

			delete clearScene;
			clearScene = nullptr;

			if (target == ClearScene::ReturnTarget::kTitle) {
				// タイトルへ戻る
				scene = Scene::kTitle;
				titleScene = new TitleScene();
				titleScene->Initialize();
			} else {
				// ステージ選択へ戻る
				scene = Scene::kStageSelect;
				stageSelectScene = new StageSelectScene();
				stageSelectScene->Initialize(stageManager);
			}
		}

		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;

	case Scene::kStageSelect:
		stageSelectScene->Update();
		break;

	case Scene::kGame:
		gameScene->Update();
		break;

	case Scene::kClear:
		clearScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;

	case Scene::kStageSelect:
		stageSelectScene->Draw();
		break;

	case Scene::kGame:
		gameScene->Draw();
		break;

	case Scene::kClear:
		clearScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE2C_26_マツモト_ソウシ_イッカく");

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
#ifdef USE_IMGUI
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	imguiManager->Initialize();
#endif


	// クラスインスタンスの生成
	stageManager = new StageManager();
	stageManager->LoadStageData();
	// デバッグ設定はステージ一覧の読み込み後に適用する。
	LoadDebugSettings();

	GameAudio::GetInstance().Initialize();

	// ステージ選択画面から開始
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}
#ifdef USE_IMGUI
		imguiManager->Begin();
#endif

		// シーン切り替え
		try {
			ChangeScene();
		} catch (const std::exception& error) {
#ifdef USE_IMGUI
			imguiManager->End();
#endif

			MessageBoxA(nullptr, error.what(), "Stage load error", MB_OK | MB_ICONERROR);

			// 読み込みに失敗したシーンを更新せず、終了処理へ進む
			break;
		}

		// 現在シーン更新
		UpdateScene();

	    // 描画開始
		dxCommon->PreDraw();

		// 現在シーン描画
		DrawScene();
#ifdef USE_IMGUI
		imguiManager->End();
		imguiManager->Draw();
#endif

		// 描画終了
		dxCommon->PostDraw();

	}

	// ゲームシーンの解放
	delete titleScene;
	delete stageSelectScene;
	delete gameScene;
	delete clearScene;
	delete stageManager;
#ifdef USE_IMGUI
	imguiManager->Finalize();
#endif

	// nullptrの代入
	titleScene = nullptr;
	gameScene = nullptr;

	// BGMを停止
	GameAudio::GetInstance().StopBgm();

	// エンジンの終了処理
	Finalize();

	return 0;
}
