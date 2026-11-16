#ifndef _CWEATHER_H
#define _CWEATHER_H

#define _MIST_STATE_START 0


class CMist
{
public:
	BOOL	m_bActive;
	BYTE	m_bMistState;	

	D3DVERTEX m_avBillboard[4];
	D3DVECTOR m_vTrans;
	D3DVECTOR m_vScale;	

private:

public:
	CMist();
	~CMist();
	BOOL Create();			// 生成雾气.	
	VOID Init();			// 雾气变量初始化。
	VOID Destory();			// 纹理等的内存释放.
	HRESULT DrawMist();
	VOID ProgressMist();	// 雾气推进。(雾气开始时刻、雾气推进中、雾气结束时刻各不相同。)
};

/*
class CRain
{
public:
	BOOL	m_bActive;
	D3DVERTEX m_avBillboard[4];
	CParticle m_xParticles;

private:

public:
	CRain();
	~CRain();
	BOOL Create();			
	VOID Init();			
	VOID Destory();			
	HRESULT DrawRain();
	VOID ProgressRain();	
};


class CSnow
{
public:
	BOOL	m_bActive;
	D3DVERTEX m_avBillboard[4];
	CParticle m_xParticles;

private:

public:
	CSnow();
	~CSnow();
	BOOL Create();			
	VOID Init();			
	VOID Destory();			
	HRESULT DrawSnow();
	VOID ProgressSnow();	
};
*/
 
class CAirWave
{
#define _MAX_X	276
#define _MAX_Y	185
#define _CURR	0
#define _NEW	1
#define _DAMP	4
public:
	BYTE	m_bDispLut[512];

	BYTE	m_bCurr, m_bNew;

public:
	CAirWave();
	~CAirWave();
	VOID	InitAirWave();
	VOID	UpdateAirWave();
	VOID	DropAirWaveMap(INT nX, INT nY, INT nW, INT nMulFactor);
	VOID	RenderAirWave();
};

#endif // _CWEATHER_H