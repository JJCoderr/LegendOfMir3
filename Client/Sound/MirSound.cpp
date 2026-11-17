// MirSound.cpp: implementation of the CMirSound class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "Common/LoginMessages.h"

#define MAX_KINDS_OF_WAVE	50
//#define MAX_DUPLE_COUNT		5
#define MAX_LIM_TIME		60000


#define MAX_MP3_WAR			-1000
#define MAX_MP3_PEACE		-100

#define _PI					3.1415927

#define MAKEPOS(a1,a2,a3)	{a1.x = a2;a1.y = a3;}

void CALLBACK AmbianceTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime );
void CALLBACK FadeInTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime );
void CALLBACK FadeOutTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime );
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
//  Name		:	CMirSound
//  Destination :	Constructor
//
//  Parameter	:	none
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
CMirSound::CMirSound()
{
	m_ppWavList = NULL;
	m_pWavListHeader = NULL;
	m_ppBuffer = NULL;
	m_3DEnable = FALSE;
	m_bPlay = TRUE;
	m_bBGMPlay = TRUE;	// 临时
	ReadWavFileList("SoundList.wwl");
	m_ppBuffer = new CSBuffer*[MAX_KINDS_OF_WAVE];
	for(int i = 0 ; i < MAX_KINDS_OF_WAVE ; i++)
	{
		m_ppBuffer[i] = new CSBuffer;
	}

	m_pMp3 = new CBMMp3;

	m_lWavMaxVol = -100;
	m_lMp3MaxVol = -1000;
	m_nUseCount = 0;
	m_bRunningTimer = FALSE;
	m_bIsWarMode = FALSE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	~CMirSound
//  Destination :	Destructor
//
//  Parameter	:	none
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
CMirSound::~CMirSound()
{
    m_lobbyOpening.Release();
    m_lobbyEffect.Release();
	unsigned int i;

	if(m_ppWavList != NULL)
	{
		for(i=0;i<m_pWavListHeader->FieldCount;i++)
		{
			delete m_ppWavList[i];
		}
	delete[] m_ppWavList;
	}

	if(m_bAmbiance)
	{
		KillTimer(g_xMainWnd.GetSafehWnd(),ID_AMBIANCE_TIMER);
		for(i = 0 ; i < MAX_AMBIANCE_COUNT ; i++)
		{
			m_pAmbianceBuffer[i]->Release();
			delete m_pAmbianceBuffer[i];
		}
	}
	m_CommonWavBuffer.Release();

	if(m_ppBuffer!=NULL)
	{
		for(int j =0;j<m_nUseCount;j++)			// Buffer Class
		{
			m_ppBuffer[j]->Release();
			delete m_ppBuffer[j];
		}
		delete[] m_ppBuffer;
	}

	if(m_pWavListHeader != NULL)	delete m_pWavListHeader;
	if(m_pMp3!=NULL) delete m_pMp3;
	if(m_pSound!=NULL)		delete m_pSound;	// Sound Class 
}


///////////////////////////////////////////////////////////////////////
//  Name		:	CalsDistance
//  Destination :	Calculate Distance from User to Sound Source
//
//  Parameter	:	Position  Src
//					Position Chr
//
//	Return Value:	int Distance
//  Update		:
///////////////////////////////////////////////////////////////////////
float CMirSound::CalsDistanceX(POINT Src,POINT Chr)		// 检查声音发生位置与当前角色之间的距离
{
	float l_Result;
	l_Result = (float)((Chr.x - Src.x)%13);						// 距离不能超过 12.
	l_Result = (float)(1.0-(float)sqrt(1.0 - l_Result/(float)12.0))*((float)-1.0);
	return m_reversePan ? -l_Result : l_Result;
}

float CMirSound::CalsDistanceY(POINT Src,POINT Chr)		// 检查声音发生位置与当前角色之间的距离
{
	float l_Result;
	l_Result = (float)((Chr.y- Src.y)%13);						// 距离不能超过 12.
	l_Result =float(1.0-(float)sqrt(1.0 - (double)(l_Result)/12.0))*((float)-1.0);
	return l_Result;
}

INT CMirSound::CalsDistance(POINT Src,POINT Chr)
{
	INT Cx,Cy;
	INT nResult;
	Cx = Src.x - Chr.x;
	Cy = Src.y - Chr.y;
	Cx = Cx<0 ? Cx*(-1) : Cx;
	Cy = Cy<0 ? Cy*(-1) : Cy;
	
	nResult = Cx >Cy ? Cx : Cy;
	return nResult;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	CalsPan
//  Destination :	Calculate Pans value
//
//  Parameter	:	int Direction
//					int Distance
//
//	Return Value:	int Pans
//  Update		:
///////////////////////////////////////////////////////////////////////
int CMirSound::CalsPan(int Dir,int Dis)	
{
	int pans;
	pans = Dir*(int)(5000.0*(sin(_PI*(Dis/26.0))));

	return m_reversePan ? -pans : pans;
}


///////////////////////////////////////////////////////////////////////
//  Name		:
//  Destination :
//
//  Parameter	:
//
//
//	Return Value:
//  Update		:
///////////////////////////////////////////////////////////////////////
int CMirSound::CalsVolume(int Dis) // 需要修改
{
	int l_Result;
	if(Dis!=0)
	{
		l_Result = (int)((cos(_PI*(Dis/26.0)))*100);
		l_Result = (101 - l_Result);		// 最大音量 0 最小音量 (-1)*x : 当前的 100-(x)%
		l_Result = m_lWavMaxVol+l_Result*(-20);			// 最小音量 -2000 ~ 最大音量 0
		return l_Result;
	}
	else
	{
		return m_lWavMaxVol;							// 重叠的或自身的声音	以最大音量
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:
//  Destination :
//
//  Parameter	:
//
//
//	Return Value:
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::ReadWavFileList(char* fName)
{
	unsigned int i;
	int Count;
	DWORD dwReadSize;
	DWORD	dwError;
	WAVELIST* l_WavList;

	HANDLE hFile = ::CreateFile(fName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);

	dwError = GetLastError();
	if(dwError==ERROR_SUCCESS)
	{
		//	Header Reading
		m_pWavListHeader = new WAVELISTHEADER;

		ReadFile(hFile,m_pWavListHeader,sizeof(WAVELISTHEADER),&dwReadSize,NULL);
		//	fclose(hFile);	
		m_ppWavList = new WAVELIST*[m_pWavListHeader->FieldCount];

		// List Reading
		Count = 0;
		for(i=0;i<m_pWavListHeader->ListCount ; i++)
		{
			l_WavList = new WAVELIST;
			ZeroMemory(l_WavList,sizeof(WAVELIST));
			ReadFile(hFile,l_WavList,sizeof(WAVELIST),&dwReadSize,NULL);

			if(l_WavList->ID != 0) 
			{
				for(unsigned int j=0; j<strlen(l_WavList->Des);j++)
				{
					char	temp[4];
					int		cmpResult;
					strncpy(temp,l_WavList->Des+j,3);
					temp[3] = NULL;
					cmpResult = strcmp(temp,"wav");
					if(cmpResult==0)
					{
						*(l_WavList->Des+j+3)=NULL;
					}
				}
				l_WavList->Des;
				m_ppWavList[Count] = l_WavList;
				Count++;
			}
			else
			{
				delete l_WavList;
			}
		}
		CloseHandle(hFile);
	}
	else
	{
		m_bPlay = FALSE;
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:
//  Destination :
//
//  Parameter	:
//
//
//	Return Value:
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::PlayMagicEffect(POINT Target, POINT Chr, int Wavnum)
{
	if(m_bPlay)
	{
		FreeNotUseBuffer();
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::PlayAmbianceWav(int WavNum)
{
	if(m_bAmbiance && m_bPlay)
	{
		FreeNotUseBuffer();
		m_pAmbianceBuffer[WavNum]->SetVolume(m_lWavMaxVol);
		m_pAmbianceBuffer[WavNum]->Play(0);
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:	PlayBkMusic
//  Destination :	Play Wave File with stream
//
//  Parameter	:	bool	looped
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::UpdateLobbyAudio(LobbyPage page, DWORD elapsed)
{
    if (page != m_lobbyPage) {
        m_lobbyOpening.Release();
        m_lobbyEffect.Release();
        if (m_lobbyPage == LobbyPage::Selection || m_lobbyPage == LobbyPage::Creation)
            m_pMp3->OnStop();
        m_lobbyPage = page;
        m_lobbyElapsed = 0;
        return;
    }
    if (page == LobbyPage::None) return;
    const DWORD delay = page == LobbyPage::Credentials ? 1200 : 1000;
    if (m_lobbyElapsed > delay) return;
    m_lobbyElapsed += elapsed;
    if (m_lobbyElapsed <= delay) return;
    if (page == LobbyPage::Credentials) {
        if (m_bPlay && m_pSound->GetDS() && SUCCEEDED(m_lobbyOpening.Load(m_pSound, "Opening.wav"))) {
            m_lobbyOpening.m_ID = 1;
            m_lobbyOpening.SetVolume(m_lWavMaxVol);
            m_lobbyOpening.Play(DSBPLAY_LOOPING);
        }
    } else if (m_bBGMPlay) {
        char file[MAX_PATH];
        GetFullPathNameA(page == LobbyPage::Selection ? "Sound\\SelChr.mp3" : "Sound\\CreateChr.mp3",
            MAX_PATH, file, NULL);
        if (m_pMp3->LoadMp3(file, m_hWnd)) m_pMp3->OnPlay(m_lMp3MaxVol, TRUE);
    }
}

void CMirSound::SetInterfaceVolume(bool music, int level)
{
    const LONG volume=min(0,40*level); // DirectSound cannot amplify above 0 dB.
    if (music) { m_lMp3MaxVol=volume; m_pMp3->SetVolume(volume); }
    else {
        m_lWavMaxVol=volume;
        m_lobbyEffect.SetVolume(volume);
        m_lobbyOpening.SetVolume(volume);
    }
}

int CMirSound::ItemSoundSample(int mode,BYTE record31,const char* name)
{
    switch (mode) {
    case 0: return 108;
    case 5: case 6: return 111;
    case 10: case 11: return 112;
    case 15: return 116;
    case 19: case 20: case 21: return 115;
    case 22: case 23: return 113;
    case 24: case 26: {
        static CLoginMessages captions; static const bool loaded=captions.Load("./CmList.dat");
        return strstr(name,captions.Get(2641).c_str()) || strstr(name,captions.Get(2642).c_str())?117:114;
    }
    case 31: return record31<106?108:118;
    default: return 118;
    }
}
void CMirSound::PlayItemEffect(int mode,BYTE record31,const char* name)
{
    const char* file=SeekFileName(ItemSoundSample(mode,record31,name));
    if (file) PlayLobbyEffect(file);
}

void CMirSound::PlayUseItemEffect(BYTE mode)
{
    if (mode>2) return;
    const char* file=SeekFileName(mode==0?108:107);
    if (file) PlayLobbyEffect(file);
}

bool CMirSound::PlayLobbyEffect(const char* file)
{
    if (!m_bPlay || !m_pSound->GetDS()) return false;
    m_lobbyEffect.Release();
    if (FAILED(m_lobbyEffect.Load(m_pSound, file))) return false;
    m_lobbyEffect.m_ID = 1;
    m_lobbyEffect.SetVolume(m_lWavMaxVol);
    return SUCCEEDED(m_lobbyEffect.Play(0));
}

HRESULT CMirSound::InspectInterfaceAudio(bool opening,DWORD& playing,LONG& volume)
{
    auto& buffer=opening?m_lobbyOpening:m_lobbyEffect;
    playing=buffer.Playing();return buffer.GetVolume(volume);
}

void CMirSound::PlayRoleVoice(bool creation, int profession, int gender)
{
    static const char* const voices[2][6] = {
        {"JMMSel.wav", "SMMSel.wav", "DMMSel.wav", "JWMSel.wav", "SWMSel.wav", "DWMSel.wav"},
        {"JMCre.wav", "SMCre.wav", "DMCre.wav", "JWCre.wav", "SWCre.wav", "DWCre.wav"}
    };
    if (profession >= 0 && profession < 3 && gender >= 0 && gender < 2)
        PlayLobbyEffect(voices[creation][profession + 3 * gender]);
}

void CMirSound::PlayBkMusicMp3(BOOL Looped,INT	nIndex)
{
	if(m_bBGMPlay)
	{
		CHAR		szMp3FileName[MAX_PATH];
		ZeroMemory(szMp3FileName,MAX_PATH);
		if(m_pMp3->IsPlaying())
		{
			// 播放中
			m_pMp3->OnStop();
		}
//		FindBGMFileName(nIndex,szMp3FileName);
		strcpy(szMp3FileName,"1.mp3");	// 临时
		if(szMp3FileName[0]!=NULL)
		{
			m_pMp3->LoadMp3(szMp3FileName,g_xMainWnd.GetSafehWnd());
			m_pMp3->OnPlay(-5000,Looped);			
			BGMFadeIn();
		}
		else
		{
			
		}
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:	StopBkMusic
//  Destination :	Stop play BackGround Sound
//
//  Parameter	:	none
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::StopBkMusicMp3(void)
{
	BGMFadeOut();
}
///////////////////////////////////////////////////////////////////////
//  Name		:	PlayActorWav
//  Destination :	Play Wave File with any effect
//
//  Parameter	:	Position Mon
//					Position Chr
//					int WavNum
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
INT CMirSound::PlayActorWav(INT Sx,INT Sy,INT Dx,INT Dy,int Wavnum,INT lVolume,BOOL bLooping)
{
	CSBuffer*	pBuffer;
	char* fname;
	POINT	Mon,Chr;
	INT Dis;
	INT Dir;
	MAKEPOS(Mon,Sx,Sy);
	MAKEPOS(Chr,Dx,Sy);
 	if(m_bPlay)
	{
		FreeNotUseBuffer();
		fname = SeekFileName(Wavnum);
		if(fname != NULL)
		{
			pBuffer= FindBuffer(Wavnum,TRUE);
			if(pBuffer!=NULL)
			{
				if(m_3DEnable)
				{
						pBuffer->PlayExtended((float)CalsDistanceX(Mon,Chr),(float)CalsDistanceY(Mon,Chr),(float)0.1,0);	// 3D pan
				}
				else
				{
					Dis = CalsDistance(Mon,Chr);
					Dir = CalsDirection(Mon,Chr);
					if(bLooping)
					{
						pBuffer->PlayExtended(DSBPLAY_LOOPING,CalsPan(Dir,Dis),CalsVolume(Dis),0);										    // 2D pan
					}
					else
					{
						lVolume = CalsVolume(Dis)*((100-lVolume==0)?1:(100-lVolume)/5);
						pBuffer->PlayExtended(0,CalsPan(Dir,Dis),lVolume,0);										    // 2D pan
					}
					return pBuffer->nBufferIdx;
				}
			}
		}
	}
	return MAX_DUPLE_COUNT;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	StopAllSound
//  Destination :	Stop All playing Sound
//
//  Parameter	:	none
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::StopAllSound(void)
{
	for(int i = 0;i<m_nUseCount;i++)
	{
		m_ppBuffer[i]->Stop();
	}
}

///////////////////////////////////////////////////////////////////////
//  Name		:	StopSound
//  Destination :	Stop play Sound
//
//  Parameter	:	int nNum
//
//	Return Value:	void
//  Update		:
///////////////////////////////////////////////////////////////////////
void CMirSound::StopSound(int nNum)
{
	CSBuffer*	pBuffer;
	pBuffer = NULL;
	pBuffer = FindBuffer(nNum,FALSE);
	if(pBuffer != NULL)
		pBuffer->Stop();
}


void CMirSound::StopSound(int nNum,int BufIdx)
{
	CSBuffer*	pBuffer;
	pBuffer = NULL;
	pBuffer = FindBuffer(nNum,FALSE);
	if(pBuffer != NULL)
		pBuffer->Stop(BufIdx);
}


///////////////////////////////////////////////////////////////////////
//  Name		:	SeekFileName
//  Destination :	Find File name with Wave number
//
//  Parameter	:	int wavnum
//
//	Return Value:	char*
//  Update		:
///////////////////////////////////////////////////////////////////////
char* CMirSound::SeekFileName(int wavnum)
{
	unsigned int i;
	if(m_bPlay)
	{
		for(i=0;i<m_pWavListHeader->FieldCount;i++)
		{
			if(wavnum == m_ppWavList[i]->ID) 
				return m_ppWavList[i]->Des;
		}
	}
	return NULL;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	FindBuffer
//  Destination :	
//
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
CSBuffer* CMirSound::FindBuffer(int nNum,BOOL ForLoad)	// 不存在时加载则为 TRUE 否则为 FASLE
{
	if(m_nUseCount>0)
	{
		for(int i=0 ; i<MAX_KINDS_OF_WAVE;i++)
		{
			if(m_ppBuffer[i]->m_ID==nNum)	// 在已读取的内容中是否存在?
			{
				return m_ppBuffer[i];
			}
			
		}
	}
	// 读取的内容中没有吗?
	if(ForLoad)
	{
		if(m_nUseCount<MAX_KINDS_OF_WAVE)
		{
			char* fname;
			// 那么开始读取!
			fname = SeekFileName(nNum);
			if(m_3DEnable) m_ppBuffer[m_nUseCount]->Enable3d();
			if(m_ppBuffer[m_nUseCount]->Load(m_pSound,fname,5)!=E_FAIL)
			{
				m_ppBuffer[m_nUseCount]->m_ID = nNum;
				m_nUseCount++;
				return m_ppBuffer[m_nUseCount-1];
			}
		}
		else
		{	// 呃! 连读取的空间都没有了吗?
			// 那就这样吧!
			return NULL;
		}
	}
	return NULL;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
BOOL CMirSound::InitMirSound(HWND hWnd)
{
	if(hWnd != NULL)
	{
		m_hWnd = hWnd;
		m_pSound = new CSound;
		return SUCCEEDED(m_pSound->Create(m_hWnd,TRUE));
	}
	return FALSE;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::Enable3D(VOID)
{
	m_3DEnable = TRUE;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
INT CMirSound::CalsDirection(POINT Src,POINT Chr)
{
	return Src.x>Chr.x ? 1 :Src.x<Chr.x ?-1:0;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
BOOL CMirSound::FreeNotUseBuffer(VOID)
{
	DWORD	dwCurrentTime;
	DWORD	dwTempTime;
	CSBuffer*	pxTempBuffer;
	int i=0;

	dwCurrentTime = timeGetTime();		// 获取当前时间。

	do
	{
		dwTempTime = m_ppBuffer[i]->GetLastUsedTime();
		if(dwTempTime!=0)
		{
			dwTempTime = dwCurrentTime - dwTempTime;
			if(dwTempTime >  MAX_LIM_TIME)	// 丢弃 1 分钟以上未使用的部分
			{	
				m_ppBuffer[i]->Release();
				pxTempBuffer = m_ppBuffer[i];
				m_nUseCount--;
				for(int j = i; j<m_nUseCount;j++)
				{
					m_ppBuffer[j] = m_ppBuffer[j+1];
				}
				m_ppBuffer[m_nUseCount] = pxTempBuffer;
				i--;
			}
			i++;
		}
	}
	while(i<m_nUseCount);
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetEnableBGM(VOID)
{
    if (!m_bBGMPlay) { m_pMp3->SetVolume(m_lMp3MaxVol); m_pMp3->Resume(); }
	m_bBGMPlay = TRUE;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetDisableBGM(VOID)
{
	m_bBGMPlay = FALSE;
    KillTimer(m_hWnd,ID_TIMER_1);
    m_bRunningTimer=FALSE;
    m_pMp3->OnStop();
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetEnablePlay(VOID)
{
	m_bPlay = TRUE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetDisablePlay(VOID)
{
	m_bPlay = FALSE;
    StopAllSound();
    m_lobbyEffect.Stop();
    m_lobbyOpening.Stop();
    m_CommonWavBuffer.Stop();
    if (m_bAmbiance)
        for (int i=0;i<MAX_AMBIANCE_COUNT;++i) m_pAmbianceBuffer[i]->Stop();
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetDisableAmbiance(VOID)
{
	if(m_bAmbiance)
	{
		m_bAmbiance = FALSE;
		for(INT i = 0 ; i < MAX_AMBIANCE_COUNT ; i++)
		{
			m_pAmbianceBuffer[i]->Release();
			delete m_pAmbianceBuffer[i];
		}

//		m_CommonWavBuffer.Release();
		if(m_CommonWavBuffer.Load(m_pSound,"10.wav",1)!=E_FAIL)
		{
			m_CommonWavBuffer.m_ID = 1;
			m_CommonWavBuffer.SetVolume(m_lWavMaxVol);
			m_CommonWavBuffer.Play(0);
		}
		KillTimer(g_xMainWnd.GetSafehWnd(),ID_AMBIANCE_TIMER);
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetEnableAmbiance(INT nMapNum,INT nCount)
{
	CHAR	szPname[MAX_PATH];
	CHAR	szFname[MAX_PATH];
	CHAR	szTname[MAX_PATH];
	if(!m_bAmbiance)
	{
		m_nAmbienceNum = nMapNum;
		m_nAmbianceCount = nCount;
//		_itoa(m_nAmbienceNum,szFname,10);
		ZeroMemory(szPname,MAX_PATH);
//		strcat(szPname,szFname);
		if(m_CommonWavBuffer.Load(m_pSound,"00.wav",1)!=E_FAIL)		// FadeIn
		{
			m_CommonWavBuffer.SetVolume(m_lWavMaxVol);
			m_CommonWavBuffer.m_ID = 1;			// 仅在环境(Ambience)情况下设为 1。
			m_CommonWavBuffer.Play(0);
		}

		for(INT i = 0 ; i < MAX_AMBIANCE_COUNT ; i++)
		{
			_itoa(i,szTname,10);
			strcat(szTname,".wav");
			strcpy(szFname,szPname);
			strcat(szFname,szTname);
			m_pAmbianceBuffer[i] = new CSBuffer;
			if(m_3DEnable)	m_pAmbianceBuffer[i]->Enable3d();
			m_pAmbianceBuffer[i]->Load(m_pSound,szFname,2);
			m_pAmbianceBuffer[i]->m_ID = 1;		// 仅在环境(Ambience)情况下设为 1。
		}
		m_bAmbiance = TRUE;
		SetTimer(m_hWnd,ID_AMBIANCE_TIMER,1000,(TIMERPROC)(AmbianceTimerProc));
	}
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
BOOL CMirSound::BGMFadeIn(LONG	lMaxVol)
{
	if(!m_bRunningTimer)
	{
		if(m_pMp3->IsPlaying())
		{
			m_lMp3MaxVol = lMaxVol;
			SetTimer(m_hWnd,ID_TIMER_1,100,(TIMERPROC)(FadeInTimerProc));
			m_bRunningTimer = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
BOOL CMirSound::BGMFadeOut(LONG lTime,LONG	lMinVol)
{
	if(!m_bRunningTimer)
	{
		if(m_pMp3->IsPlaying())
		{
			lTime = (lTime*2)/100;
			m_lMp3MaxVol = lMinVol;
			SetTimer(m_hWnd,ID_TIMER_1,lTime,(TIMERPROC)(FadeOutTimerProc));
			m_bRunningTimer = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
HRESULT CMirSound::MessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
		case WM_DSHOW_NOTIFY:
			{
				m_pMp3->MessageProcess();
				break;
			}
	}
	return 0l;
}




///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::EndFading(INT nState)
{
	if(nState==0)
	{
		// End of Fade in
		KillTimer(g_xMainWnd.GetSafehWnd(),ID_TIMER_1);
		//
		
	}
	else
	{
		// End of Fade out
		if(m_lMp3MaxVol<=(-5000)) m_pMp3->OnStop();
		KillTimer(g_xMainWnd.GetSafehWnd(),ID_TIMER_1);
		//
	}
	m_bRunningTimer = FALSE;
}




///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetInWarMode(VOID)
{
	if(!m_bIsWarMode)
	{
		m_lMp3MaxVol =(-2000);
		BGMFadeOut(m_lMp3MaxVol);
		m_bIsWarMode = TRUE;
	}
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
VOID CMirSound::SetInPeaceMode(VOID)
{
	if(m_bIsWarMode)
	{
		m_lMp3MaxVol =(0);
		BGMFadeIn(m_lMp3MaxVol);
		m_bIsWarMode = FALSE;
	}
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
// MAX = 100 Min = 1
BOOL CMirSound::SetMasterVol(INT nVolume)
{
	if(nVolume>=1 && nVolume<=100)
	{
		m_lWavMaxVol = (nVolume-101)*(50);		// 100 -> -50    0 -> -5000
		m_lMp3MaxVol = (m_lWavMaxVol-1)*(10);		// Wav volume 的 -10db
		return TRUE;
	}
	return FALSE;
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
// MAX = 100 Min = 1
BOOL CMirSound::SetBGMVol(INT nVolume)
{
	if(nVolume>=0 && nVolume<=100)
	{
		m_lMp3MaxVol = (m_lWavMaxVol-1)*(10)*(nVolume+1);		// Wav volume 的 -10dB 占百分之几？
		return TRUE;
	}
	return FALSE;
}


///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
BOOL CMirSound::SetWavVol(INT nVolume)
{
	return SetMasterVol(nVolume);
}



///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
void CALLBACK FadeInTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
	LONG	lVolume;
	lVolume = g_xSound.m_pMp3->GetVolume();
	// if End of Fade-out SendMessage to Window WM_USER_ENDTIMER
	if(lVolume>=(g_xSound.m_lMp3MaxVol))
		SendMessage(g_xMainWnd.GetSafehWnd(),WM_USER_ENDTIMER,(WPARAM)0,NULL);
	lVolume = lVolume + 100;
	g_xSound.m_pMp3->SetVolume(lVolume);
}

///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
void CALLBACK FadeOutTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime )
{
	LONG	lVolume;
	lVolume = g_xSound.m_pMp3->GetVolume();
	// if End of Fade-out SendMessage to Window WM_USER_ENDTIMER
	if(lVolume<(g_xSound.m_lMp3MaxVol))
		SendMessage(g_xMainWnd.GetSafehWnd(),WM_USER_ENDTIMER,(WPARAM)1,NULL);
	lVolume = lVolume - 100;
	g_xSound.m_pMp3->SetVolume(lVolume);
}




///////////////////////////////////////////////////////////////////////
//  Name		:	
//  Destination :	
//
//  Parameter	:	
//
//	Return Value:	
//  Update		:
///////////////////////////////////////////////////////////////////////
void CALLBACK AmbianceTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime )
{
	static	INT nCount = 11;
	static	INT	nReply = 0;
	INT nNum;
	INT	Fact_1,Fact_2;
	Fact_1 = (rand())%10+1;
	Fact_2 = (rand())%10+1;

	nCount++;
	
	if((Fact_1+Fact_2)<=6)  // 以 15% 以上的概率 Play   1~15% = 15%
		nNum = 1;
	else
		if((Fact_1+Fact_2)<=8)  // 以 28% 以上的概率 Play  16~28% = 12%
			nNum = 2;
		else
			if((Fact_1+Fact_2)<=10)  // 45% 以上的概率 Play   27~45%  = 18%
				nNum = 3;
			else
				if((Fact_1+Fact_2)<=11)  // 55% 以上的概率 Play   46~55% = 10%
					nNum = 4;
				else
					if((Fact_1+Fact_2)<=13)  // 以 72% 以上的概率 Play  56~72% = 16%
						nNum = 5;
					else
						if((Fact_1+Fact_2)<=16)  // 以 90% 以上的概率 Play	72~90 = 18%
							nNum = 7;
						else
							nNum = 8;			 //	10%

	if(nCount>=13)
	{
		nNum = 0;
		nCount = 0;
		nReply++;
	}
	if(nNum >= 0 && nNum <= 8)
	{
		g_xSound.PlayAmbianceWav(nNum);		// 原本
	}
	if(nReply>=g_xSound.m_nAmbianceCount)
	{
		nReply = 0;
		g_xSound.SetDisableAmbiance();
		return;
	}
}

BOOL CMirSound::ChgPlayingSet(INT nWavIdx,INT nBufferIdx,INT Sx,INT Sy,INT Dx,INT Dy)
{
	POINT	Mon,Chr;
	INT Dis;
	INT Dir;
	CSBuffer*	pBuffer;
	MAKEPOS(Mon,Sx,Sy);
	MAKEPOS(Chr,Dx,Sy);

	pBuffer= FindBuffer(nWavIdx,FALSE);
	if(pBuffer!=NULL)
	{
		Dis = CalsDistance(Mon,Chr);
		Dir = CalsDirection(Mon,Chr);
		pBuffer->Pause(nBufferIdx);
		pBuffer->SetPan(nBufferIdx,CalsPan(Dir,Dis));
		pBuffer->SetVolume(nBufferIdx,CalsVolume(Dis));
		pBuffer->Continue(nBufferIdx);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
void CMirSound::StopSceneAudio()
{
    StopAllSound();m_lobbyEffect.Stop();m_lobbyOpening.Stop();m_CommonWavBuffer.Stop();
    if(m_bAmbiance) for(int i=0;i<MAX_AMBIANCE_COUNT;++i) m_pAmbianceBuffer[i]->Stop();
    StopBkMusicMp3();UpdateLobbyAudio(LobbyPage::None,0);
}
