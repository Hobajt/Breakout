#include "breakout/sound.h"

#include "breakout/log.h"

namespace Sound {

	void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
		Device& dev = Device::Get();

		std::lock_guard quad(dev.mutex);
		if (dev.decoder == nullptr) {
			ma_event_signal(&dev.stopSignal);
			return;
		}

		ma_uint64 framesRead = ma_decoder_read_pcm_frames(dev.decoder, pOutput, frameCount);
		if (framesRead < frameCount || dev.terminating) {
			ma_event_signal(&dev.stopSignal);
		}

		(void)pInput;
	}

	void Play(Audio& audio) {
		Device::Get().Play(audio.decoder);
		LOG(LOG_INFO, "Playing '%s'\n", audio.filepath.c_str());
	}

	//==== Device ====

	Device& Device::Get() {
		static Device d = Device();
		return d;
	}

	void Device::Play(ma_decoder* decoder_) {
		std::lock_guard guard(mutex);
		ma_decoder_seek_to_pcm_frame(decoder_, 0);
		decoder = decoder_;
	}

	Device::Device() {
		//device setup & init
		deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format = ma_format_f32;;
		deviceConfig.playback.channels = 0;
		deviceConfig.sampleRate = 0;
		deviceConfig.dataCallback = data_callback;
		deviceConfig.pUserData = nullptr;

		if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
			LOG(LOG_ERROR, "Audio - Failed to open playback device.\n");
			throw std::exception();
		}

		//sound thread init
		soundThread = std::thread(
			[this]() {
				while (!terminating) {
					//new decoder queued & not playing -> start the device
					if (!playing && decoder != nullptr) {
						Start();
						printf("STARTING\n");
					}

					if (playing) {
						//await device termination signal (on audio end)
						ma_event_wait(&stopSignal);
						printf("Done playing\n");

						//stop the device
						ma_device_stop(&device);
						decoder = nullptr;
						playing = false;
					}
				}
			}
		);

		LOG(LOG_CTOR, "[C] Audio device\n");
	}

	Device::~Device() {
		terminating = true;
		soundThread.join();

		ma_device_uninit(&device);
		LOG(LOG_DTOR, "[D] Audio device\n");
	}

	void Device::Start() {
		ma_event_init(&stopSignal);

		if (ma_device_start(&device) != MA_SUCCESS) {
			LOG(LOG_ERROR, "Failed to start playback device.\n");
		}
		else {
			playing = true;
		}
	}

	//==== Audio ====

	Audio::Audio(const std::string& filepath_) : filepath(filepath_), valid(false) {
		decoder = new ma_decoder();

		ma_result result = ma_decoder_init_file(filepath.c_str(), NULL, decoder);
		if (result != MA_SUCCESS) {
			LOG(LOG_ERROR, "Failed to load audio from '%s'.\n", filepath.c_str());
			delete decoder;
			throw std::exception();
		}

		LOG(LOG_RESOURCE, "Loaded audio file from '%s'.\n", filepath.c_str());
		LOG(LOG_CTOR, "[C] Audio '%s'\n", filepath.c_str());
		valid = true;
	}

	Audio::~Audio() {
		if (valid && decoder != nullptr) {
			ma_decoder_uninit(decoder);
			delete decoder;
			LOG(LOG_DTOR, "[D] Audio '%s'\n", filepath.c_str());
		}
	}

	Audio::Audio(Audio&& a) noexcept {
		valid = a.valid;
		filepath = a.filepath;
		decoder = a.decoder;

		a.decoder = nullptr;
		a.valid = false;
	}

	Audio& Audio::operator=(Audio&& a) noexcept {
		valid = a.valid;
		filepath = a.filepath;
		decoder = a.decoder;

		a.decoder = nullptr;
		a.valid = false;

		return *this;
	}

}//namespace Sound
