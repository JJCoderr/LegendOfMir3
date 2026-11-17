#include "StdAfx.h"
#include "SceneLighting.h"


CLightFog::CLightFog()
{
	Init(RGB(220, 220, 220));
}


CLightFog::~CLightFog()
{
}


VOID CLightFog::Init(DWORD dwLightColor)
{
	m_dwLightColor = dwLightColor;

	for ( INT nCntY = 0; nCntY < (NUM_CNTY_LIGHTTILE+1); nCntY++ )
	{
		for ( INT nCntX = 0; nCntX < (NUM_CNTX_LIGHTTILE+1); nCntX++ )
		{
			m_avLight[nCntY][nCntX] = D3DLVERTEX(D3DVECTOR((FLOAT)(nCntX*TILE_WIDTH), (FLOAT)(-nCntY*TILE_HEIGHT), 0),
								  			     m_dwLightColor, RGB(0, 0, 0),  0, 0); 
		}
	}

	memcpy(m_avSaveLight, m_avLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));

    ClearSavedLightColor();
}


VOID CLightFog::ChangeLightColor(DWORD dwColor)
{
	if ( m_dwLightColor != dwColor )
	{
		for ( INT nCntY = 0; nCntY < NUM_CNTY_LIGHTTILE+1; nCntY++ )
		{
			for ( INT nCntX = 0; nCntX < NUM_CNTX_LIGHTTILE+1; nCntX++ )
			{
				m_avLight[nCntY][nCntX].color = dwColor;
			}
		}
		m_dwLightColor = dwColor;
		memcpy(m_avSaveLight, m_avLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));
	}
}


VOID CLightFog::ClearSavedLightColor()
{
	memcpy(m_avLight, m_avSaveLight, sizeof(D3DLVERTEX)*(NUM_CNTY_LIGHTTILE+1)*(NUM_CNTX_LIGHTTILE+1));
    for(int row=0;row<NUM_CNTY_LIGHTTILE;++row) for(int column=0;column<NUM_CNTX_LIGHTTILE;++column) {
        const WORD a=WORD(column+(NUM_CNTX_LIGHTTILE+1)*row),b=a+1,d=a+NUM_CNTX_LIGHTTILE+1,c=d+1;
        const WORD triangles[]={a,b,d,b,c,d};
        memcpy(m_pwLightIndices+(column+row*NUM_CNTX_LIGHTTILE)*6,triangles,sizeof(triangles));
    }

}


BOOL CLightFog::GetScreenPosToLightTile(INT nX, INT nY, INT* nTileX, INT* nTileY)
{
	*nTileX = (nX - TILE_START_XPOS) / TILE_WIDTH;
	*nTileY = (nY - TILE_START_YPOS) / TILE_HEIGHT;
	
	return TRUE;
}


BOOL CLightFog::SetLightRadiusWithRing(INT x,INT y,INT innerRadius,
    INT innerRed,INT innerGreen,INT innerBlue,INT outerRadius,INT outerRed,INT outerGreen,INT outerBlue)
{
    const mir3::SceneLight light{x,y,innerRadius,{innerRed,innerGreen,innerBlue},outerRadius,{outerRed,outerGreen,outerBlue}};
    const int cx=(x+8)/48,cy=(y+29)/32;
    for(int row=max(0,cy-outerRadius);row<=min(NUM_CNTY_LIGHTTILE,cy+outerRadius);++row)
        for(int column=max(0,cx-outerRadius);column<=min(NUM_CNTX_LIGHTTILE,cx+outerRadius);++column) {
            const auto value=light.sample(column,row);auto& color=m_avLight[row][column].color;
            const int red=min(255,int((color>>16)&255)+value[0]);
            const int green=min(255,int((color>>8)&255)+value[1]);
            const int blue=min(255,int(color&255)+value[2]);
            color=(red<<16)|(green<<8)|blue;
            if(column==NUM_CNTX_LIGHTTILE || row==NUM_CNTY_LIGHTTILE) continue;
            const WORD a=WORD(column+(NUM_CNTX_LIGHTTILE+1)*row),b=a+1,d=a+NUM_CNTX_LIGHTTILE+1,c=d+1;
            WORD* indices=m_pwLightIndices+(column+row*NUM_CNTX_LIGHTTILE)*6;
            if(light.diagonal_top_left_to_bottom_right(column,row)) {
                const WORD triangle[]={a,b,c,a,c,d};memcpy(indices,triangle,sizeof(triangle));
            } else {
                const WORD triangle[]={a,b,d,b,c,d};memcpy(indices,triangle,sizeof(triangle));
            }
        }
    return TRUE;
}

BOOL CLightFog::SetLightRadiusWithCircle(INT XLightPos, INT YLightPos, INT nRadius, 
										 INT nRedValue, INT nGreenValue, INT nBlueValue, 
										 FLOAT rDrakRate)
{
    if(rDrakRate==1.0f) return SetLightRadiusWithRing(XLightPos,YLightPos,0,0,0,0,nRadius,nRedValue,nGreenValue,nBlueValue);

	INT nX, nY;
	GetScreenPosToLightTile(XLightPos, YLightPos, &nX, &nY);

	INT nStartTileX			= nX - nRadius;
	INT nStartTileY			= nY - nRadius;
	INT nEndTileX			= nX + nRadius + 1;
	INT nEndTileY			= nY + nRadius + 1;

	// 以 y 为轴计算最暗的部分.(因为 y 比 x 更小)
	nRadius = (nRadius*TILE_HEIGHT);

	DOUBLE rLengthCnt		= 0;
	INT nSelectedRedValue	= 0; 
	INT nSelectedGreenValue	= 0;
	INT nSelectedBlueValue	= 0;

	if ( nStartTileX < 0 )					nStartTileX = 0;
	if ( nStartTileY < 0 )					nStartTileY = 0;
	if ( nEndTileX > NUM_CNTX_LIGHTTILE+1 )	nEndTileX = NUM_CNTX_LIGHTTILE+1;
	if ( nEndTileY > NUM_CNTY_LIGHTTILE+1 )	nEndTileY = NUM_CNTY_LIGHTTILE+1;	

	for ( INT nCntY = nStartTileY; nCntY < nEndTileY; nCntY++ )
	{
		for ( INT nCntX = nStartTileX; nCntX < nEndTileX; nCntX++ )
		{			
			rLengthCnt	= sqrt((double)(XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS))*(XLightPos-(nCntX*TILE_WIDTH+TILE_START_XPOS)) + 
							   (YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS))*(YLightPos-(nCntY*TILE_HEIGHT+TILE_START_YPOS)));
			if ( nCntX < NUM_CNTX_LIGHTTILE && nCntY < NUM_CNTY_LIGHTTILE )
			{
				// 不含点坐标的左下. 不含点坐标的右上.
				if ( (nCntX > nX && nCntY < nY) || (nCntX < nX && nCntY > nY) )
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
				}
				else
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
				}
			}

/*			RECT rc;
			SetRect(&rc,
				   (nCntX*TILE_WIDTH+TILE_START_XPOS),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
				   (nCntX*TILE_WIDTH+TILE_START_XPOS+TILE_WIDTH),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS+TILE_HEIGHT));
			g_xMainWnd.DrawWithGDI(rc, NULL, RGB(0, 255, 255), 1);
*/
			if ( rLengthCnt	<= nRadius )
			{
				nSelectedRedValue	= nRedValue - INT((nRedValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedGreenValue	= nGreenValue - INT((nGreenValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedBlueValue	= nBlueValue - INT((nBlueValue*rLengthCnt*rDrakRate)/(nRadius));
/*				SetRect(&rc,
					   (nCntX*TILE_WIDTH+TILE_START_XPOS),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
					   (nCntX*TILE_WIDTH+TILE_START_XPOS+2),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS+2));
				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 0), 1);
*/			}
			else
			{
				nSelectedRedValue = nSelectedGreenValue = nSelectedBlueValue = 0;
			}

			if ( nSelectedRedValue	 < 0 )		nSelectedRedValue	= 0;
			if ( nSelectedGreenValue < 0 )		nSelectedGreenValue = 0;
			if ( nSelectedBlueValue	 < 0 )		nSelectedBlueValue	= 0;

			if ( m_avLight[nCntY][nCntX].color == 0 )
				m_avLight[nCntY][nCntX].color = RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue); 
			else
			{
				nSelectedRedValue += (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].color);
				nSelectedGreenValue += (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].color);
				nSelectedBlueValue += (INT)RGB_GETRED(m_avLight[nCntY][nCntX].color);

				if ( nSelectedRedValue	 > 255 )		nSelectedRedValue	= 255;
				if ( nSelectedGreenValue > 255 )		nSelectedGreenValue = 255;
				if ( nSelectedBlueValue	 > 255 )		nSelectedBlueValue	= 255;

				m_avLight[nCntY][nCntX].color =  RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue);
			}
		}
	}

	return TRUE;
}


BOOL CLightFog::SetLightRadiusWithElipse(INT XLightPos, INT YLightPos, INT nXRadius, INT nYRadius, 
										 INT nRedValue, INT nGreenValue, INT nBlueValue, 
										 FLOAT rDrakRate)
{
	INT nX, nY;
	GetScreenPosToLightTile(XLightPos, YLightPos, &nX, &nY);

	INT nStartTileX			= nX - nXRadius;
	INT nStartTileY			= nY - nYRadius;
	INT nEndTileX			= nX + nXRadius + 1;
	INT nEndTileY			= nY + nYRadius + 1;
	
	FLOAT rXRadius = (FLOAT)(nXRadius*TILE_WIDTH);
	FLOAT rYRadius = (FLOAT)(nYRadius*TILE_HEIGHT);
	INT	  nRadius  = 0;

	if ( rXRadius > rYRadius )
	{
		nRadius = (INT)rXRadius;
	}
	else
	{
		nRadius = (INT)rYRadius;
	}

	DOUBLE rLengthCnt		= 0;
	INT nSelectedRedValue	= 0; 
	INT nSelectedGreenValue	= 0; 
	INT nSelectedBlueValue	= 0; 

	if ( nStartTileX < 0 )					nStartTileX = 0;
	if ( nStartTileY < 0 )					nStartTileY = 0;
	if ( nEndTileX > NUM_CNTX_LIGHTTILE+1 )	nEndTileX = NUM_CNTX_LIGHTTILE+1;
	if ( nEndTileY > NUM_CNTY_LIGHTTILE+1 )	nEndTileY = NUM_CNTY_LIGHTTILE+1;


	for ( INT nCntY = nStartTileY; nCntY < nEndTileY; nCntY++ )
	{
		for ( INT nCntX = nStartTileX; nCntX < nEndTileX; nCntX++ )
		{
			if ( nCntX < NUM_CNTX_LIGHTTILE && nCntY < NUM_CNTY_LIGHTTILE )
			{
				// 不含点坐标的左下. 不含点坐标的右上.
				if ( (nCntX > nX && nCntY < nY) || (nCntX < nX && nCntY > nY) )
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
				}
				else
				{
					WORD* pwIndex = ((WORD*)m_pwLightIndices+(nCntX+nCntY*(NUM_CNTX_LIGHTTILE))*6);
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));
					*pwIndex++ = nCntX+((NUM_CNTX_LIGHTTILE+1)*(nCntY+1));  
					*pwIndex++ = (nCntX+1)+((NUM_CNTX_LIGHTTILE+1)*nCntY);  
					*pwIndex++ = (nCntX)+((NUM_CNTX_LIGHTTILE+1)*nCntY);
				}
			}

/*			RECT rc;
			SetRect(&rc,
				   (nCntX*TILE_WIDTH+TILE_START_XPOS),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
				   (nCntX*TILE_WIDTH+TILE_START_XPOS+TILE_WIDTH),
				   (nCntY*TILE_HEIGHT+TILE_START_YPOS+TILE_HEIGHT));
			g_xMainWnd.DrawWithGDI(rc, NULL, RGB(0, 255, 255), 1);
*/
	
			INT nCurrCntXRate, nCurrCntYRate;
			if ( rXRadius > rYRadius )		
			{
				nCurrCntXRate = nCntX;
				nCurrCntYRate = nCntY + (INT)((nCntY-nY)*(FLOAT)(rXRadius/rYRadius-1.0f));
			}
			else if ( rXRadius < rYRadius )										
			{
				nCurrCntXRate = nCntX + (INT)((nCntX-nX)*(FLOAT)(rYRadius/rXRadius-1.0f));
				nCurrCntYRate = nCntY;
			}
			else
			{
				nCurrCntXRate = nCntX;
				nCurrCntYRate = nCntY;
			}

			rLengthCnt	= sqrt((double)(XLightPos-(nCurrCntXRate*TILE_WIDTH+TILE_START_XPOS))*(XLightPos-(nCurrCntXRate*TILE_WIDTH+TILE_START_XPOS)) + 
							   (YLightPos-(nCurrCntYRate*TILE_HEIGHT+TILE_START_YPOS))*(YLightPos-(nCurrCntYRate*TILE_HEIGHT+TILE_START_YPOS)));

/*			SetRect(&rc,
				   (XLightPos),
				   (YLightPos),
				   (XLightPos+5),
				   (YLightPos+5));
			g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 0), 1);
*/
			if ( rLengthCnt	<= nRadius )
			{
				nSelectedRedValue	= nRedValue - INT((nRedValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedGreenValue	= nGreenValue - INT((nGreenValue*rLengthCnt*rDrakRate)/(nRadius));
				nSelectedBlueValue	= nBlueValue - INT((nBlueValue*rLengthCnt*rDrakRate)/(nRadius));
	/*			SetRect(&rc,
					   (nCntX*TILE_WIDTH+TILE_START_XPOS),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS),
					   (nCntX*TILE_WIDTH+TILE_START_XPOS+2),
					   (nCntY*TILE_HEIGHT+TILE_START_YPOS+2));
				g_xMainWnd.DrawWithGDI(rc, NULL, RGB(255, 0, 0), 1);
	*/			}
			else
			{
				nSelectedRedValue = nSelectedGreenValue = nSelectedBlueValue = 0;
			}

			if ( nSelectedRedValue	 < 0 )		nSelectedRedValue	= 0;
			if ( nSelectedGreenValue < 0 )		nSelectedGreenValue = 0;
			if ( nSelectedBlueValue	 < 0 )		nSelectedBlueValue	= 0;

			if ( m_avLight[nCntY][nCntX].color == 0 )
				m_avLight[nCntY][nCntX].color = RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue); 
			else
			{
				if ( nSelectedRedValue < (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].color) )
					nSelectedRedValue = (INT)RGB_GETBLUE(m_avLight[nCntY][nCntX].color);
				if ( nSelectedGreenValue < (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].color) )
					nSelectedGreenValue = (INT)RGB_GETGREEN(m_avLight[nCntY][nCntX].color);
				if ( nSelectedBlueValue < (INT)RGB_GETRED(m_avLight[nCntY][nCntX].color) )
					nSelectedBlueValue = (INT)RGB_GETRED(m_avLight[nCntY][nCntX].color);

				m_avLight[nCntY][nCntX].color =  RGB(nSelectedRedValue, nSelectedGreenValue, nSelectedBlueValue);
			}
		}
	}

	return TRUE;
}




VOID CLightFog::ShowLightFog(WORD wHalfScrnWidth, WORD wHalfScrnHeight)
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()) )
		{
			D3DVECTOR vTrans;
			D3DMATRIX matTrans;
			D3DMATRIX matWorldOriginal;

			g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
			
			vTrans.x = -wHalfScrnWidth; vTrans.y = wHalfScrnHeight; vTrans.z = 0;
			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matTrans);
			g_xMainWnd.Get3DDevice()->SetTexture(0, NULL);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
			g_xMainWnd.Get3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
														  D3DFVF_LVERTEX,
														  m_avLight, NUM_LIGHTTILE_VERTICES, 
														  m_pwLightIndices, NUM_LIGHTTILE_INDICES, NULL );
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->EndScene();
		}
	}
}
