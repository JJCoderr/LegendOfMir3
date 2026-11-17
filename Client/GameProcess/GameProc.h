/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#ifndef _GAMEPROC_H
#define	_GAMEPROC_H
#include "LevelEffects.h"
#include "SceneLighting.h"
#include "SceneWeather.h"
#include "ExitFlow.h"
#include "SpellPlayback.h"
#include "StagedSpell.h"
#include "GroundSpell.h"
#include "SpellParticlePool.h"
#include "SpellOrbit.h"
#include "SpellDirection.h"
#include <map>



typedef struct tagMEMMAPPEDID
{
	CActor*			pxActor[_MAX_CELL_OBJ];
	CMagic*			pxMagic[_MAX_CELL_OBJ];
	LPGROUNDITEM	pstItem[GroundItemDisplay::CellCapacity];
}MEMMAPPEDID, *LPMEMMAPPEDID;



class CGameProcess : public CWHDefProcess
{
protected:
	VOID	OnSvrMsgNewMap(_TDEFAULTMESSAGE	*ptdm, char *pszMapName);
	VOID	OnSvrMsgLogon(_TDEFAULTMESSAGE	*ptdm, char *pszMsg);
	VOID	OnSvrMsgHear(_TDEFAULTMESSAGE *ptdm, char *pszMsg);

public:
    mir3::SceneAmbient m_sceneAmbient;
    void EnterSceneAmbient(WORD mode);
    void AdvanceSceneAmbient(DWORD elapsed);
    void LightSceneMap();
    void LightSceneActors();
    ExitFlow::Guard m_exitGuard;
    ExitFlow::Transition m_exitTransition;
    bool CheckExitAllowed(DWORD now);
    bool ConfirmExit(int action,DWORD now);
    void ReturnToSelection();
    void DrawExitTransition();
    CLevelEffects m_levelEffects;
    struct NativeSpell {
        mir3::SpellPlayback playback;
        std::optional<mir3::StagedSpell> staged;
        std::optional<mir3::SpellOrbit> orbit;
        unsigned orbitDirection=0;
        std::int64_t soundVoice=0;
        int soundSample=0;
        mir3::SpellFlight::Point anchor={};
        bool groundWave=false,ember=false;
        POINT cell={};
        int remaining=0,tint=255;
        DWORD target=0;std::uint64_t targetInstance=0,serial=0;
    };
    std::uint64_t m_nextNativeSpell=1;
    std::vector<NativeSpell> m_nativeSpells;
    bool StartNativeSpell(CActor& source,WORD skill,POINT target,DWORD targetId);
    std::int64_t PlaySpellSound(int sample,POINT cell);
    void StartGroundSpell(mir3::MagicEffectId effect,POINT source,POINT target,int facing);
    void StartSpellEmber(POINT cell,int resourceId);
    void AdvanceNativeSpells(DWORD elapsed,std::uint64_t before);
    struct SpellMark {POINT cell;int frame;DWORD age=0;};
    static constexpr DWORD SpellMarkLifetime=2000,SpellEmberLifetime=10000;
    static constexpr int FireMark=220,IceMark=260,FirstEmber=330,LastEmber=334;
    std::vector<SpellMark> m_spellMarks;
    mir3::SpellParticlePool m_spellParticles;
    mir3::SnowWeather m_weather;
    bool m_weatherEnabled=true;
    std::map<WORD,POINT> m_spellLibraryOffsets;
    void ClearNativeSpells();
    void DrawSpellMarks();
    void DrawSpellParticles();
    void DrawNativeSpells();
    void LightNativeSpells();
    WORD m_weaponTraining=0;
    bool ReceiveProgression(const _TDEFAULTMESSAGE& header);
    bool ReceiveAbility(const _TDEFAULTMESSAGE& header,const char* encoded);
	 CGameProcess();
	~CGameProcess();
public:
	BOOL					m_bStatusView;
	BOOL					m_bTileView;
	BOOL					m_bCellView;
	BOOL					m_bObj1View;
	BOOL					m_bObj2View;
	BOOL					m_bStopPtView;
    void SetShadowBlend(bool enabled);
	BOOL					m_bShadowABlend;
	BOOL					m_bShowFPS;

	CHAR					m_szMapName[MAX_PATH];
	MEMMAPPEDID				m_stMemMappedID[_VIEW_CELL_Y_COUNT][_VIEW_CELL_X_COUNT];

	CPDLList<CActor>		m_xActorList;
	CPDLList<CMagic>		m_xMagicList;
	CPDLList<CMagic>		m_xGroundMagicList;
	CPDLList<GROUNDITEM>	m_stMapItemList;

	CBMMp3					m_xBGM;
	CParticle				m_xParticles;
	CImageHandler			m_xImage;
	CMapHandler				m_xMap;
	CInterface				m_xInterface;
	CMyHero					m_xMyHero;
	HBRUSH					m_hBrush;
	POINT					m_ptMousePos;
	POINT					m_ptCurrTile;
	BOOL					m_bRender;
	BYTE					m_bDayState;
	BYTE					m_bFogState;
	CLightFog				m_xLightFog;
	CFlyingTail				m_xFlyingTail;
	CSmoke					m_xSmoke;

	CElec					m_xElec;

	CNoticeBox				m_xNotice;

	CActor*					m_pxMouseTargetActor;
	CActor*					m_pxMouseOldTargetActor;
	CActor*					m_pxSavedTargetActor;

	CWHQueue				m_xWaitPacketQueue;	
	CWHQueue				m_xWaitDefaultPacketQueue;	

	WORD					m_wInputDelayTime;
	WORD					m_wCurrInputDelayTime;

	FLOAT					m_fDarkRate;

	FLOAT					m_fTileLightRed;
	FLOAT					m_fTileLightGreen;
	FLOAT					m_fTileLightBlue;

	WORD					m_wShadowClr[800];

	D3DMATRIX				m_matWorld;

	BOOL					m_bShowShine;	
	DWORD					m_dwLastShineTick;
	WORD					m_wShineCurrDelay;
	BYTE					m_bCurrShineFrame;
	LPDIRECTDRAWSURFACE7	m_lpddsShineTextr[10];		// 物品闪烁.
	D3DVERTEX				m_avBillboard[4];
	BOOL					RenderShine(INT nLoopTime);

	WORD					m_wNPCWndPosX, m_wNPCWndPosY;

	virtual LRESULT DefMainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void				OnProcPacketNotEncode(char *pszMsg);

	void				OnSocketMessageRecieve(char *pszMsg);

	virtual VOID		OnConnectToServer();
	virtual char*		OnMessageReceive(CHAR* pszMessage);

	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnSysKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnSysKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMsgInputted(WPARAM wParam, LPARAM lParam);
	LRESULT OnWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDoubleClick(WPARAM wParam, LPARAM lParam);
	LRESULT OnTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnDestroy(WPARAM wParam, LPARAM lParam);

public:
	VOID	Init();
	VOID	Load();
	VOID	DeleteProc();
	VOID	SetNextProc();

	VOID	DropItemShow();
	VOID	LoadMapChanged(CHAR* szMap);

	VOID	AutoTargeting();

	BOOL	ReceivePacket(CHAR* szMsg, INT nPacketLen);
	VOID	RenderObject(INT nLoopTime);
	VOID	RenderScene(INT nLoopTime);
    void UpdateTargetDisplay(CActor* actor,DWORD now);

	BOOL	CheckMove(INT nX, INT nY, INT nSpeed, POINT* ptDestTile = NULL, INT* nDestDir = NULL);
	VOID	CheckMappedData(INT nLoopTime, BOOL bIsMoveTime);

	VOID	RenderMapTileGrid();
	VOID	CommandAnalyze();

	BOOL	TargetInLongAttack(BYTE bDir);

	VOID	ProcessPacket();
	VOID	ProcessDefaultPacket();

	VOID	CloseNPCWindow();

	virtual VOID ShowStatus(INT nStartX, INT nStartY);

	CActor*	FindActor(int nID); // gadget

};



#endif //_GAMEPROC_H