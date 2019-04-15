#pragma once

#include <CustomLibrary/CustomSDL/C_Engine.h>

namespace ctl
{
	class Audio
	{
	public:
		Audio()
		{
			//Get capture device count
			m_recDevices = SDL_GetNumAudioDevices(SDL_TRUE);

			//No recording devices
			if (m_recDevices < 1)
				throw Error(SDL_GetError());

			//Cap recording device count
			if (m_recDevices > MAX_RECORDING_DEVICES)
				m_recDevices = MAX_RECORDING_DEVICES;
		}

		//Initilizes 1 audio devices at certain number
		void initAudio(const int &index, const int &sec)
		{
			//Default audio spec
			SDL_AudioSpec wantRecSpec;
			SDL_zero(wantRecSpec);
			wantRecSpec.freq = 44100;
			wantRecSpec.format = AUDIO_F32;
			wantRecSpec.channels = 2;
			wantRecSpec.samples = 4096;
			wantRecSpec.userdata = this;
			wantRecSpec.callback = audioRecordingCallback;

			//Open recording device
			m_recDeviceId = SDL_OpenAudioDevice(deviceName(index).c_str(), SDL_TRUE, &wantRecSpec, &m_recSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

			//Device failed to open
			if (m_recDeviceId == 0)
				throw Error(SDL_GetError());

			//Default audio spec
			SDL_AudioSpec wantPlaySpec;
			SDL_zero(wantPlaySpec);
			wantPlaySpec.freq = 44100;
			wantPlaySpec.format = AUDIO_F32;
			wantPlaySpec.channels = 2;
			wantPlaySpec.samples = 4096;
			wantPlaySpec.userdata = this;
			wantPlaySpec.callback = audioPlaybackCallback;

			//Open playback device
			m_playDeviceId = SDL_OpenAudioDevice(nullptr, SDL_FALSE, &wantPlaySpec, &m_playSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

			//Device failed to open
			if (m_playDeviceId == 0)
				throw Error(SDL_GetError());

			//Calculate bytes per second
			const int bytesPerSecond = wantRecSpec.freq * wantRecSpec.channels * (SDL_AUDIO_BITSIZE(wantRecSpec.format) / 8);

			//Calculate buffer size
			m_buffByteSize = (sec + 1) * bytesPerSecond;
			//Calculate max buffer size
			m_buffByteMaxPos = sec * bytesPerSecond;

			//Allocate and initialize byte buffer
			m_recordingBuff = new Uint8[m_buffByteSize];
		}

		//Get device name at certain number (UTF-8)
		std::string deviceName(const int &id)
		{
			if (m_recDevices < id)
				std::cerr << "Invalid audio device: " << id << std::endl;

			return SDL_GetAudioDeviceName(id, SDL_TRUE);
		}

		//Start recording
		void startRec()
		{
			if (SDL_GetAudioDeviceStatus(m_recDeviceId) == SDL_AUDIO_PAUSED)
			{
				//Go back to beginning to buffer
				m_buffBytePos = 0;

				//Reset memory
				std::memset(m_recordingBuff, 0, m_buffByteSize);

				//Start recording
				SDL_PauseAudioDevice(m_recDeviceId, SDL_FALSE);
			}
		}

		//Start playing back
		void startPlay()
		{
			if (SDL_GetAudioDeviceStatus(m_recDeviceId) == SDL_AUDIO_PAUSED)
			{
				//Go back to beginning of buffer
				m_buffBytePos = 0;

				//Start playback
				SDL_PauseAudioDevice(m_playDeviceId, SDL_FALSE);
			}
		}

		//Pause device
		void pause(const SDL_AudioDeviceID &id)
		{
			//Stop recording audio
			SDL_PauseAudioDevice(id, SDL_TRUE);
		}

		//Check if recorded enougth
		bool checkByte(const SDL_AudioDeviceID &id)
		{
			//Lock callback
			SDL_LockAudioDevice(id);

			//Check for finish
			bool temp = m_buffBytePos > m_buffByteMaxPos;

			//Unlock callback
			SDL_UnlockAudioDevice(id);

			return temp;
		}

		~Audio()
		{
			//Deallocate memory
			if (m_recordingBuff != nullptr)
				delete[] m_recordingBuff;
		}

		constexpr const int& getNumOfDevices() const { return m_recDevices; }

		constexpr const SDL_AudioDeviceID& getRecID() const { return m_recDeviceId; }
		constexpr const SDL_AudioDeviceID& getPlayID() const { return m_playDeviceId; }

	private:
		//Maximum number of supported recording devices
		static constexpr int MAX_RECORDING_DEVICES = 10;

		//Number of available devices
		int m_recDevices = 0;

		//Recieved audio spec
		SDL_AudioSpec m_recSpec;
		SDL_AudioSpec m_playSpec;

		//Recording data buffer
		Uint8 *m_recordingBuff = nullptr;
		//Size of data buffer
		Uint32 m_buffByteSize;
		//Position in data buffer
		Uint32 m_buffBytePos;
		//Maximum position in data buffer for recording
		Uint32 m_buffByteMaxPos;

		//Audio device IDs
		SDL_AudioDeviceID m_recDeviceId = 0;
		SDL_AudioDeviceID m_playDeviceId = 0;

		//Recording/playback callbacks
		static void audioRecordingCallback(void *userdata, Uint8 *stream, int length)
		{
			//Copy audio from stream
			std::memcpy(&static_cast<Audio*>(userdata)->m_recordingBuff[static_cast<Audio*>(userdata)->m_buffBytePos], stream, length);

			//Move along buffer
			static_cast<Audio*>(userdata)->m_buffBytePos += length;
		}
		static void audioPlaybackCallback(void *userdata, Uint8 *stream, int length)
		{
			//Copy audio to stream
			std::memcpy(stream, &static_cast<Audio*>(userdata)->m_recordingBuff[static_cast<Audio*>(userdata)->m_buffBytePos], length);

			//Move along buffer
			static_cast<Audio*>(userdata)->m_buffBytePos += length;
		}
	};
}