
#include "main.h"
#include "audio.h"

// BGM: MP3 support via Media Foundation (Windows' own built-in decoders --
// no extra library/asset needed, same reasoning as Hud using DirectWrite
// instead of a bitmap font). Only LoadMp3() below uses these.
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <vector>
#include <string.h> // strrchr / _stricmp (extension check in Load())
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

IXAudio2* Audio::m_Xaudio = NULL;
IXAudio2MasteringVoice* Audio::m_MasteringVoice = NULL;

void Audio::InitMaster()
{
	// COM初期化
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Media Foundation初期化：BGMのMP3読み込み用（LoadMp3()を参照）
	MFStartup(MF_VERSION);

	// XAudio生成
	XAudio2Create(&m_Xaudio, 0);

	// マスタリングボイス生成
	m_Xaudio->CreateMasteringVoice(&m_MasteringVoice);
}


void Audio::UninitMaster()
{
	m_MasteringVoice->DestroyVoice();
	m_Xaudio->Release();
	MFShutdown();
	CoUninitialize();
}

void Audio::Load(const char* FileName)
{

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	//拡張子で分岐: .mp3 なら Media Foundation 経由の LoadMp3() へ、
	// それ以外(.wav 前提)は元々の mmio 読み込みのまま。
	const char* ext = strrchr(FileName, '.');
	bool isMp3 = ext && (_stricmp(ext, ".mp3") == 0);

	if (isMp3)
	{
		bool ok = LoadMp3(FileName, wfx);
		assert(ok); // Load()の他の失敗経路(mmioOpen等)と同じくassertで気付けるようにしてある

		m_Xaudio->CreateSourceVoice(&m_SourceVoice, &wfx);
		assert(m_SourceVoice);
		return;
	}

	{
		HMMIO hmmio = NULL;
		MMIOINFO mmioinfo = { 0 };
		MMCKINFO riffchunkinfo = { 0 };
		MMCKINFO datachunkinfo = { 0 };
		MMCKINFO mmckinfo = { 0 };
		UINT32 buflen;
		LONG readlen;


		hmmio = mmioOpen((LPSTR)FileName, &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
		{
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		}
		else
		{
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);



		buflen = datachunkinfo.cksize;
		m_SoundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)m_SoundData, buflen);


		m_Length = readlen;
		m_PlayLength = readlen / wfx.nBlockAlign;


		mmioClose(hmmio, 0);
	}


	// サウンドソース生成
	m_Xaudio->CreateSourceVoice(&m_SourceVoice, &wfx);
	assert(m_SourceVoice);
}

bool Audio::LoadMp3(const char* FileName, WAVEFORMATEX& outWfx)
{
	HRESULT hr;

	// MFCreateSourceReaderFromURL wants a wide path -- same narrow(Shift-
	// JIS/ACP)->wide conversion the project already uses in hud.cpp.
	wchar_t widePath[MAX_PATH]{};
	MultiByteToWideChar(CP_ACP, 0, FileName, -1, widePath, MAX_PATH);

	IMFSourceReader* reader = nullptr;
	hr = MFCreateSourceReaderFromURL(widePath, NULL, &reader);
	if (FAILED(hr)) return false;

	IMFMediaType* partialType = nullptr;
	MFCreateMediaType(&partialType);
	partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = reader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, partialType);
	partialType->Release();
	if (FAILED(hr)) { reader->Release(); return false; }

	IMFMediaType* actualType = nullptr;
	hr = reader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &actualType);
	if (FAILED(hr)) { reader->Release(); return false; }

	WAVEFORMATEX* wfxPtr = nullptr;
	UINT32 wfxSize = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(actualType, &wfxPtr, &wfxSize);
	actualType->Release();
	if (FAILED(hr)) { reader->Release(); return false; }

	outWfx = *wfxPtr; // wfxSize can exceed sizeof(WAVEFORMATEX) for exotic layouts, but a plain stereo/mono 16-bit PCM mp3 (the normal case) fits -- fine for BGM
	CoTaskMemFree(wfxPtr);

	reader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, FALSE);
	reader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

	std::vector<BYTE> pcm;
	pcm.reserve(1 << 20); // 1MB head start; grows automatically past that

	for (;;)
	{
		DWORD flags = 0;
		IMFSample* sample = nullptr;
		hr = reader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &flags, NULL, &sample);
		if (FAILED(hr)) break;
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
		if (!sample) continue; // gap in the stream -- keep reading

		IMFMediaBuffer* buffer = nullptr;
		if (SUCCEEDED(sample->ConvertToContiguousBuffer(&buffer)))
		{
			BYTE* data = nullptr;
			DWORD dataLen = 0;
			if (SUCCEEDED(buffer->Lock(&data, NULL, &dataLen)))
			{
				pcm.insert(pcm.end(), data, data + dataLen);
				buffer->Unlock();
			}
			buffer->Release();
		}
		sample->Release();
	}

	reader->Release();

	if (pcm.empty()) return false;

	m_SoundData = new unsigned char[pcm.size()];
	memcpy(m_SoundData, pcm.data(), pcm.size());
	m_Length = (int)pcm.size();
	m_PlayLength = outWfx.nBlockAlign ? (m_Length / outWfx.nBlockAlign) : 0;

	return true;
}

void Audio::Uninit()
{
	m_SourceVoice->Stop();
	m_SourceVoice->DestroyVoice();

	delete[] m_SoundData;
}
void Audio::Play(bool Loop)
{
	m_SourceVoice->Stop();
	m_SourceVoice->FlushSourceBuffers();


	// バッファ設定
	XAUDIO2_BUFFER bufinfo;

	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = m_Length;
	bufinfo.pAudioData = m_SoundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = m_PlayLength;

	// ループ設定
	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = m_PlayLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	m_SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

	// 再生
	m_SourceVoice->Start();

}