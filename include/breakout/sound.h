#pragma once

#include <string>

#include <miniaudio.h>
#include <thread>
#include <mutex>

namespace Sound {

	class Device {
	public:
		static Device& Get();

		void Play(ma_decoder* decoder);
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

	void Play(Audio& audio);

	int KOKOTINA(const char* lsakdhjfgzdkj);

}//namespace Sound
