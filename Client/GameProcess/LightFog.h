/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/



#ifndef _CLIGHTFOG_H
#define _CLIGHTFOG_H

class CLightFog
{
public:
	CLightFog();
	~CLightFog();
protected:
	D3DTLVERTEX			m_avLight[(NUM_CNTY_LIGHTTILE+1)][(NUM_CNTX_LIGHTTILE+1)];
	D3DTLVERTEX			m_avSaveLight[(NUM_CNTY_LIGHTTILE+1)][(NUM_CNTX_LIGHTTILE+1)];
	WORD				m_pwLightIndices[NUM_LIGHTTILE_INDICES];
	DWORD				m_dwLightColor;	

public:
    DWORD ColorAt(int column,int row) const {return m_avLight[row][column].color;}
    bool DiagonalAt(int column,int row) const {
        return m_pwLightIndices[(column+row*NUM_CNTX_LIGHTTILE)*6+2]==column+(NUM_CNTX_LIGHTTILE+1)*row+NUM_CNTX_LIGHTTILE+2;
    }
	VOID	Init(DWORD dwLightColor);
	VOID	ShowLightFog();
	// 将新变更的颜色保存到 m_avSaveLight 中.
	VOID	ChangeLightColor(DWORD dwColor);
	VOID	ClearSavedLightColor();
	BOOL	GetScreenPosToLightTile(INT nX, INT nY, INT* nTileX, INT* nTileY);
	BOOL	SetLightRadiusWithRing(INT XLightPos, INT YLightPos, INT nSmallRadius, 
								   INT nSmallRed, INT nSmallGreen, INT nSmallBlue, 
								   INT nLargeRadius,
								   INT nLargeRed, INT nLargeGreen, INT nLargeBlue);
	BOOL	SetLightRadiusWithCircle(INT XLightPos, INT YLightPos, INT nRadius,
									 INT nRedValue, INT nGreenValue, INT nBlueValue, 
									 FLOAT rDrakRate = 1.0f);
	BOOL	SetLightRadiusWithElipse(INT XLightPos, INT YLightPos, INT nXRadius, INT nYRadius, 
		                             INT nRedValue, INT nGreenValue, INT nBlueValue, 
									 FLOAT rDrakRate = 1.0f);
};


#endif // _CLIGHTFOG_H