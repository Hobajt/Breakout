#pragma once

#include <string>

#include <miniaudio.h>
#include <thread>
#include <mutex>

#include <memory>

namespace Sound {

	struct Audio;
	using AudioRef = std::shared_ptr<Audio>;

	class Device {
	public:
		static Device& Get();

		void Play(AudioRef& audio);
	private:
		Device();
		~Device();

		void Start();
	public:
		ma_device device;
		ma_device_config deviceConfig;
		ma_event stopSignal;

		ma_decoder* decoder = nullptr;
		bool playing = false;

		std::mutex mutex;
		std::mutex sleepMutex;
		std::thread soundThread;
		bool terminating = false;
		bool del = false;

		AudioRef currAudio = nullptr;
	};

	struct Audio {
		bool valid = false;
		std::string filepath;

		ma_decoder* decoder = nullptr;
	public:
		Audio(const std::string& filepath);

		Audio() = default;
		~Audio();

		Audio(const Audio&) = delete;
		Audio& operator=(const Audio&) = delete;

		Audio(Audio&&) noexcept;
		Audio& operator=(Audio&&) noexcept;
	};

	//ma_decoder Load(const std::string& filepath);

	void Play(AudioRef& audio);

	void Release();

}//namespace Sound
