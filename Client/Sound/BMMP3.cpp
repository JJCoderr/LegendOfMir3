// BMMP3.cpp: implementation of the BMMP3 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBMMp3::CBMMp3()
{
	CoInitialize(NULL);
	m_pBasicAudio = NULL;
	m_pMediaControl= NULL;
	m_pMediaSeeking= NULL;
	m_pMediaPosition= NULL;
	m_pMediaEventEx= NULL;
	m_pGraphBuilder= NULL;
	m_bIsPlaying = FALSE;		// Is this Mp3 Media playing?
	m_bInited = FALSE;			// Is this class initialized?
	m_bLooping = FALSE;			// Is this Mp3 Media Looping Play?
	m_hWnd = NULL;				// Handle of this class's owner
	m_bIsLoadMp3 = FALSE;
}

CBMMp3::~CBMMp3()
{
	Release();
	CoUninitialize();
}

BOOL CBMMp3::InitBMMp3(HWND hWnd)
{
    Release();
    m_hWnd = hWnd;
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
        IID_IGraphBuilder, (void**)&m_pGraphBuilder);
    if (SUCCEEDED(hr)) hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&m_pMediaControl);
    if (SUCCEEDED(hr)) hr = m_pGraphBuilder->QueryInterface(IID_IBasicAudio, (void**)&m_pBasicAudio);
    if (SUCCEEDED(hr)) hr = m_pGraphBuilder->QueryInterface(IID_IMediaSeeking, (void**)&m_pMediaSeeking);
    if (SUCCEEDED(hr)) hr = m_pGraphBuilder->QueryInterface(IID_IMediaEventEx, (void**)&m_pMediaEventEx);
    if (SUCCEEDED(hr)) hr = m_pGraphBuilder->QueryInterface(IID_IMediaPosition, (void**)&m_pMediaPosition);
    if (FAILED(hr)) { Release(); return FALSE; }
    m_bInited = TRUE;
    return TRUE;
}

BOOL CBMMp3::LoadMp3(CHAR* szFileName, HWND hWnd)
{
    // Copy before releasing: callers may pass the stored filename.
    CHAR filename[MAX_PATH];
    lstrcpynA(filename, szFileName, MAX_PATH);
    if (!InitBMMp3(hWnd)) return FALSE;
    WCHAR wide[MAX_PATH];
    if (!MultiByteToWideChar(CP_ACP, 0, filename, -1, wide, MAX_PATH) ||
        FAILED(m_pGraphBuilder->RenderFile(wide, NULL))) {
        Release();
        return FALSE;
    }
    lstrcpyA(m_szFileName, filename);
    m_bIsLoadMp3 = TRUE;
    return TRUE;
}

BOOL CBMMp3::OnPlay(BOOL Looping)
{
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(m_bIsPlaying)
		{
			OnStop();
			m_bLooping = Looping;
			return OnPlay(Looping);
		}
		m_bLooping = Looping;
		if(FAILED(m_pMediaControl->Run()))
			return FALSE;
		SetNotifyWindow(WM_DSHOW_NOTIFY,0x00);
		m_bIsPlaying = TRUE;
	}
	else
		return FALSE;
	return TRUE;
}

BOOL CBMMp3::OnPlay(LONG lVolume,BOOL Looping)
{
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(m_bIsPlaying)
		{
			OnStop();
			m_bLooping = Looping;
			return OnPlay(Looping);
		}
		m_bLooping = Looping;
		SetVolume(lVolume);
		if(FAILED(m_pMediaControl->Run()))
			return FALSE;

		SetNotifyWindow(WM_DSHOW_NOTIFY,0x00);
		m_bIsPlaying = TRUE;
	}
	else
		return FALSE;
	return TRUE;
}
BOOL CBMMp3::OnStop(VOID)
{
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(m_bIsPlaying)
		{
			SetNotifyFlags(AM_MEDIAEVENT_NONOTIFY);	// 不产生 WM_DSHOW_NOTIFY
			if(FAILED(m_pMediaControl->Stop()))
				return FALSE;
			m_pMediaPosition->put_CurrentPosition(0);	// 重新回到开头.
			m_bIsPlaying = FALSE;
// Keep the loaded graph available for replay; Release owns destruction.
		}
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CBMMp3::OnPause(VOID)
{
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(m_bIsPlaying)
		{
			if(FAILED(m_pMediaControl->Pause()))
				return FALSE;
			m_bIsPlaying = TRUE;
		}
		else
			return FALSE;
	}
	return TRUE;
}


HRESULT CBMMp3::MessageProcess(VOID)
{
    if (!m_pMediaEventEx) return S_FALSE;
    LONG code;
    LONG_PTR first, second;
    while (SUCCEEDED(m_pMediaEventEx->GetEvent(&code, &first, &second, 0))) {
        m_pMediaEventEx->FreeEventParams(code, first, second);
        if (code == EC_COMPLETE && m_bLooping) {
            m_pMediaPosition->put_CurrentPosition(0);
            return m_pMediaControl->Run();
        }
        if (code == EC_COMPLETE || code == EC_WINDOW_DESTROYED) {
            OnStop();
            return S_OK;
        }
    }
    return S_OK;
}

VOID CBMMp3::Release(VOID)
{
	if(m_bIsPlaying)
		OnStop();
	if(m_pMediaPosition!=NULL)
	{
		m_pMediaPosition->Release();
		m_pMediaPosition = NULL;
	}
	if(m_pMediaEventEx!=NULL)
	{
		m_pMediaEventEx->Release();
		m_pMediaEventEx = NULL;
	}
	if(m_pMediaSeeking!=NULL)
	{
		m_pMediaSeeking->Release();
		m_pMediaSeeking = NULL;
	}
	if(m_pBasicAudio!=NULL)
	{
		m_pBasicAudio->Release();
		m_pBasicAudio = NULL;
	}
	if(m_pMediaControl!=NULL)
	{
		m_pMediaControl->Release();
		m_pMediaControl = NULL;
	}
	if(m_pGraphBuilder!=NULL)
	{
		m_pGraphBuilder->Release();
		m_pGraphBuilder= NULL;
	}
	m_bInited = FALSE;
	m_bIsLoadMp3 = FALSE;
}

BOOL CBMMp3::SetVolume(LONG lVolume)
{
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(FAILED(m_pBasicAudio->put_Volume(lVolume)))
			return FALSE;
	}
	return TRUE;
}

LONG CBMMp3::GetVolume(VOID)
{
	LONG	lResult = 0;;
	if(m_bInited&&m_bIsLoadMp3)
	{
		if(m_bIsPlaying)
		{
			m_pBasicAudio->get_Volume(&lResult);
		}
	}
	return lResult;
}

////////////////////////////////////////////////////////////////
// Set Message that is Event occur
// lMsg : Long
// cOnOff : BYTE  (ex) On = 0x00     Off = 0x01
/////////////////////////////////////////////////////////////////
BOOL CBMMp3::SetNotifyWindow(LONG lMsg,LONG lOnOff)
{	
	if(FAILED(m_pMediaEventEx->SetNotifyWindow((OAHWND)m_hWnd, lMsg ,0))) return FALSE;
	if(SetNotifyFlags(lOnOff)) return FALSE;
	return TRUE;
}

BOOL CBMMp3::SetNotifyFlags(LONG lFlag)
{
	if(FAILED(m_pMediaEventEx->SetNotifyFlags(lFlag))) return FALSE;
	return TRUE;
}
