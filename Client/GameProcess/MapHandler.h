/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#ifndef _MAPHANDLER_
#define	_MAPHANDLER_


#pragma pack(1)
typedef struct tagLIGHTINFO
{
	BOOL	bIsLight;
	CHAR	cLightSizeType;
	CHAR	cLightColorType;
}LIGHTINFO, *LPLIGHTINFO;

// 保存地图文件加载相关信息.
// 保存当前所在区域全部地图的相关事项。
typedef struct tagMAPFILEHEADER
{
	CHAR	szDesc[20];
	WORD	wAttr;
	SHORT	shWidth;
	SHORT	shHeight;
	CHAR	cEventFileIdx;
	CHAR	cFogColor;
}MAPFILEHEADER, *LPMAPFILEHEADER;

typedef struct tagTILEINFO
{
	BYTE	bFileIdx;
	WORD	wTileIdx;
}TILEINFO, *LPTILEINFO;

typedef struct tagCELLINFO
{
	BYTE	bFlag;
	BYTE	bObj1Ani;
	BYTE	bObj2Ani;
	BYTE	bFileIdx;
	WORD	wObj1;
	WORD	wObj2;
	BYTE	bDoorIdx;
	BYTE	bDoorOffset;
	WORD	wLigntNEvent;
}CELLINFO, *LPCELLINFO;
/*
typedef struct tagDOORIMAGEINFO
{
	WORD	wPosX;
	WORD	wPosY;
	WORD	wImageNum;
}DOORIMAGEINFO, *LPDOORIMAGEINFO;

typedef struct tagDOORINFO
{
	BYTE			bDoorIdx;
	BYTE			bDoorImgCnt;
	LPDOORIMAGEINFO	pstDoorImgInfo;
}DOORINFO, *LPDOORINFO;
*/
#pragma pack(8)


class CMapHandler
{
private:
protected:
public:
	LPTILEINFO		m_pstTileInfo;
	LPCELLINFO		m_pstCellInfo;

	MAPFILEHEADER	m_stMapFileHeader;				// 已加载地图文件的头.

	// 与角色移动相关.
	SHORT			m_shStartViewTileX;				// 视图区域的起始 X Tile 坐标.(比实际屏幕起始图块坐标稍大一些.)
	SHORT			m_shStartViewTileY;				// 视图区域比实际屏幕区域横、竖各多留 1 个图块。
	SHORT			m_shViewOffsetX;				// 沿 X 轴滚动的长度.(角色行走动作的帧.)
	SHORT			m_shViewOffsetY;				

	RECT			m_rcView;						// 地图基础图块将要绘制的区域.(用于屏幕裁剪)

	POINT			m_ptMovePixelArray[6];			// 用于移动的像素值保存数据.

	WORD			m_wSavedMapBuffer[_VIEW_CELL_X_COUNT*_CELL_WIDTH*_VIEW_CELL_Y_COUNT*_CELL_HEIGHT];				// 记录基本图块和 SM 图块的缓冲区. 自己的角色移动之前不会更新.


	DWORD			m_dwAniSaveTime[8];
	BYTE			m_bAniTileFrame[8][16];

	CWHWilImageData*	m_pxTileImg[_MAX_TILE_FILE];
/*
	BYTE			m_bDoorCount;
	LPDOORINFO		m_pstDoorInfo;
	BYTE*			m_pbCellIDoorIdx;
*/

public:
	CMapHandler();
	~CMapHandler();
	VOID			InitMapHandler();				// 类信息初始化.
	VOID			LoadMapHandler(CHAR* szMapFile, CImageHandler* pxImgHandler, INT nX, INT nY);// 加载图像及各种地图数据.
	VOID			DestroyMapHandler();			// 类信息初始化及内存释放。

	BOOL			LoadMapData(CHAR* szMapFile);	// 加载所应用地图的数据。(每次地图改变时都会被调用。)
	VOID			LoadMapImage(CImageHandler* pxImgHandler);					// 加载地图相关的图像数据(使用内存映射)。
	VOID			LoadNewMapBuffer();				// 地图首次加载时，把最初的内容应用到图块、SM图块记录缓冲区。

	VOID			FreeMapData();					// 释放所应用地图的数据.(每次地图变化时被调用.)

	VOID			DrawBaseTile(INT nX, INT nY);	// 绘制基本图块。
//	VOID			DrawOpenDoor(INT nX, INT nY);
//	VOID			SetDoorIndex();

	VOID			ScrollMap(INT nCharDir, INT nCharFrame, INT nSpeed);		// 角色移动时应用地图的 Scroll 相关变量。
	VOID			ScrollMap(INT nCharDir, WORD wCurrDelay, WORD wMoveDelay, INT nSpeed);
	VOID			SetMovedTileBuffer(SHORT shMovedTileX, SHORT shMovedTileY);	// 角色移动时应用新更新的默认图像(图块, SM图块).
	VOID			SetStartViewTile(INT nX, INT nY);							// 绘制视图区域的初始值设置.

	BOOL			GetNextTileCanMove(INT nXPos, INT nYPos, BYTE bDir, INT nGap, POINT* lpptTarget);

/////////////////////////////////////////////////////////////////////////////////////////////////////
	VOID			GetScrnPosFromTilePos(SHORT shTileX, SHORT shTileY, SHORT& shScrnX, SHORT& shScrnY);
	VOID			GetTilePosFromScrnPos(SHORT shScrnX, SHORT shScrnY, SHORT& shTileX, SHORT& shTileY);

	BYTE			CalcDirection16(WORD wFireTileX, WORD wFireTileY, WORD wDestTileX, WORD wDestTileY);
	BYTE			CalcDirection8(WORD wFireTileX, WORD wFireTileY, WORD wDestTileX, WORD wDestTileY);


//  获取当前图块地图属性的函数。
	BYTE			GetTileAttribute(INT nX, INT nY);
	BYTE			GetEventNum(INT nX, INT nY);
//	BOOL			GetDoorState(INT nX, INT nY);

	INT				GetDoor(INT nX, INT nY);
	VOID			OpenDoor(INT nX, INT nY, INT nIdx);
	VOID			CloseDoor(INT nX, INT nY, INT nIdx);
	INT				GetDoorImgIdx(INT nX, INT nY);
	BOOL			IsDoorOpen(INT nX, INT nY);

	LIGHTINFO		GetTileLightInfo(INT nX, INT nY);
//	VOID			SetDoorState(BYTE bIndex, BOOL bDoorOpen);
	VOID			SetAniTileFrame(INT nLoopTime);
};


#endif //_MAPHANDLER_
























//	VOID			DrawOneCellObjTile(INT nX, INT nY);
//	BOOL			DrawOneCellOverObjTile(INT nX, INT nY);

