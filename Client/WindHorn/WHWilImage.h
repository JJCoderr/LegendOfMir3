/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#ifndef _WILIMAGEHANDLER_
#define	_WILIMAGEHANDLER_



/******************************************************************************************************************

	CWilImageData Class Declaration

   将威尔图像加载到内存,或以文件内存映射形式设置到所需位置.

*******************************************************************************************************************/


#pragma pack(1)

typedef struct tagWILFILEHEADER
{
	CHAR	szTmp[40];
	INT		nImageCount;
}WILFILEHEADER, *LPWILFILEHEADER;

typedef struct tagWILFILEIMAGEINFO
{
	SHORT	shWidth;
	SHORT	shHeight;
	SHORT	shPX;
	SHORT	shPY;
	DWORD	dwImageLength;
}WILIMAGEINFO, *LPWILIMAGEINFO;

typedef struct tagWIXFILEIMAGEINFO
{
	CHAR	szTmp[40];
	INT		nIndexCount;
	INT*	pnPosition;
}WIXIMAGEINFO, *LPWIXIMAGEINFO;


typedef struct tagNEWWILFILEHEADER
{
	SHORT	shComp;
	CHAR	szTitle[20];
	SHORT	shVer;
	INT		nImageCount;
}NEWWILFILEHEADER, *LPNEWWILFILEHEADER;

typedef struct tagNEWWILFILEIMAGEINFO
{
	SHORT	shWidth;
	SHORT	shHeight;
	SHORT	shPX;
	SHORT	shPY;
	CHAR	bShadow;					
	SHORT	shShadowPX;
	SHORT	shShadowPY;
	DWORD	dwImageLength;
}NEWWILIMAGEINFO, *LPNEWWILIMAGEINFO;
typedef struct tagNEWWIXFILEIMAGEINFO
{
	CHAR	szTitle[20];
	INT		nIndexCount;
	INT*	pnPosition;
}NEWWIXIMAGEINFO, *LPNEWWIXIMAGEINFO;

#pragma pack(8)


class CWHWilImageData
{
private:
protected:
	BOOL				m_bIsMemMapped;				// 是否使用内存映射.
	BOOL				m_bIsCompressed;			// 是否压缩.

	LPBYTE				m_pbStartData;				// 数据的起始地址。(整个数据区域与文件大小相同。)

	INT					m_nCurrImageIdx;			// 当前指向的索引.


public:
	WIXIMAGEINFO		m_stWixImgaeInfo;			// Wix 文件数据。
	NEWWIXIMAGEINFO		m_stNewWixImgaeInfo;

	LPWILIMAGEINFO		m_lpstCurrWilImageInfo;		// 指向当前 wil 文件信息的指针。
	LPNEWWILIMAGEINFO	m_lpstNewCurrWilImageInfo;	

	BYTE*				m_pbCurrImage;				// 当前 WIL 文件的图像指针.
	TCHAR				m_szWilFileName[MAX_PATH];

	CWHWilImageData();
	~CWHWilImageData();

	BOOL NewLoad(CHAR* szWilFile, BOOL bIsMemMapped = TRUE, BOOL bComp = TRUE);
	BOOL NewSetIndex(DWORD dwIndex);				// 使用外部传入的索引，Setting 指向当前图像信息的地址.


	BOOL Load(CHAR* szWilFile, BOOL bIsMemMapped = TRUE, BOOL bComp = TRUE);
	VOID Init();
	VOID Destroy();
	VOID SetIndex(INT nIndex);						// 使用外部传入的索引，Setting 指向当前图像信息的地址.
};


#endif //_WILIMAGEHANDLER_
