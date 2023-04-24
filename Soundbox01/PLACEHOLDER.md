		Local void DummyFunctionForLoadingMusic(HWND self)
		{
			// Separate out the load of the music and the change of text into something better.
			auto Text = Win32Caption(self);
			auto Index = ListBox_GetCurSel(MusicList);
			auto Selection = ListBox_GetCurSel(MusicList);
			Local int PreviousIndex;
			// TODO: figure out how to release the buffer and reload...
			wchar_t IndexOut[64] = { 0 };
			swprintf(IndexOut, 64, L"Index %d\n", Index);
			OutputDebugString(IndexOut);
			if (Index != nPos && voice1 != nullptr)
			{
				OutputDebugString(L"We have to reload the music...\n");
				HRESULT hr = voice1->FlushSourceBuffers();

				if (SUCCEEDED(hr))
				{
					// ReloadMusic();

				}

			}

			if (Index != -1 && wcscmp(Text->c_str(), L"Play\0") == 0)
			{

				auto TextLen = ListBox_GetTextLen(MusicList, Index);
				std::wstring Text;
				Text.resize(TextLen);
				ListBox_GetText(MusicList, Index, Text.data());

				SetWindowTextW(self, L"Pause");
				HRESULT hr = S_OK;
				// Ask for current selection of MusicList

				OutputDebugStringW(L"");

				if (voice1 == nullptr)
				{
					auto Data = LoadWaveMMap(&WaveFormatEx, Text.data());

					if (Data.has_value())
					{

						if (SUCCEEDED(hr))
						{
							OutputDebugString(L"CreateSourceVoice\n");
							hr = audio->CreateSourceVoice(&voice1, &WaveFormatEx, 0, XAUDIO2_DEFAULT_FREQ_RATIO);
						}

						if (Data->Location)
						{
							OutputDebugStringW(L"Try and play the note");
							auto XBuffer = LoadBuffer(Data.value());
							voice1->SetVolume(g_Volume, XAUDIO2_COMMIT_NOW);
							hr = voice1->SubmitSourceBuffer(XBuffer.get());
							MusicLoaded = true;
				
							SubmitThreadpoolWork(WorkItem);
						}


						if (FAILED(hr))
						{
							DWORD dwError = GetLastError();
							if (dwError != 0x00)
							{
								printf("0x%x\n", dwError);
								printf("failed to create Voice\n");
							}
						}
					}
				}

				if (voice1 && MusicLoaded)
				{
					if (SUCCEEDED(hr))
					{
						hr = voice1->Start();
					}
				}
			}
			else
			{
				if (!MusicLoaded)
					return;
				SetWindowTextW(self, L"Play");

				voice1->Stop();
			}

		}


---

/*
This is ThreadPool creation
*/
WorkItem = CreateThreadpoolWork(TaskHandler, nullptr, nullptr);
	if (WorkItem == nullptr)
	{
		OutputDebugStringW(L"Failed to create work item\n");
		return -2;
	}


	hEvent = CreateEvent(nullptr, true, false, nullptr);


void NTAPI
TaskHandler(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	wchar_t Buf[64] = { 0 };
	wsprintf(Buf, L"Starting new work\n");
	OutputDebugStringW(Buf);
	memset(Buf, 0, sizeof(wchar_t) * 64);
	wsprintf(Buf, L"WaveFormatEx information %d; Size=%zd\n", WaveFormatEx.nChannels, g_Data.has_value() ? g_Data->WaveSize : 0);
	OutputDebugString(Buf);

}