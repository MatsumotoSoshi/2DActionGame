#pragma once
#include "KamataEngine.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

class GameAudio {
public:
	enum class Bgm : std::size_t { kTitle, kStageSelect, kGame, kClear };

	static GameAudio& GetInstance() {
		static GameAudio instance;
		return instance;
	}

	void Initialize() {
		if (initialized_) {
			return;
		}

		auto* audio = KamataEngine::Audio::GetInstance();

		const std::array<const char*, 4> paths = {"BGM/titleBGM.wav", "BGM/stageSelectBGM.wav", "BGM/gameSceneBGM.wav", "BGM/clearSceneBGM.wav"};

		for (std::size_t i = 0; i < paths.size(); ++i) {
			const std::filesystem::path fullPath = std::filesystem::absolute(std::filesystem::path("Resources") / paths[i]);

			if (!std::filesystem::is_regular_file(fullPath)) {
				throw std::runtime_error("BGM file not found: " + fullPath.string());
			}

			bgmSounds_[i] = audio->LoadWave(paths[i]);
		}

		confirmSound_ = audio->LoadWave("SE/selectSE.wav");
		dashSound_ = audio->LoadWave("SE/dashSE.wav");
		breakSound_ = audio->LoadWave("SE/break.wav");

		initialized_ = true;
	}

	void PlayBgm(Bgm bgm) {
		assert(initialized_);

		// 前のBGMを停止
		StopBgm();

		// 新しいBGMをループ再生
		bgmVoice_ = KamataEngine::Audio::GetInstance()->PlayWave(bgmSounds_[static_cast<std::size_t>(bgm)], true, 0.25f);

		bgmPlaying_ = true;
	}

	void StopBgm() {
		if (!bgmPlaying_) {
			return;
		}

		KamataEngine::Audio::GetInstance()->StopWave(bgmVoice_);
		bgmPlaying_ = false;
	}

	void PlayConfirm() {
		assert(initialized_);

		KamataEngine::Audio::GetInstance()->PlayWave(confirmSound_, false, 0.6f);
	}

	void PlayDash() {
		assert(initialized_);

		KamataEngine::Audio::GetInstance()->PlayWave(dashSound_, false, 0.5f);
	}

	void PlayBreak() {
		assert(initialized_);

		KamataEngine::Audio::GetInstance()->PlayWave(breakSound_, false, 0.6f);
	}

private:
	GameAudio() = default;
	GameAudio(const GameAudio&) = delete;
	GameAudio& operator=(const GameAudio&) = delete;

	std::array<uint32_t, 4> bgmSounds_{};

	uint32_t confirmSound_ = 0;
	uint32_t dashSound_ = 0;
	uint32_t breakSound_ = 0;

	// 読み込みハンドルと再生ハンドルは別
	uint32_t bgmVoice_ = 0;

	bool initialized_ = false;
	bool bgmPlaying_ = false;
};