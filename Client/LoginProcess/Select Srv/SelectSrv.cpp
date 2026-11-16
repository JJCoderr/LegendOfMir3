#include "stdafx.h"

CSelectSrv::CSelectSrv():CBMWnd(), m_ppxSrvBtn(NULL), m_nSrvCount(0), m_pressed(-1), m_hover(-1)
{
    D3DVECTOR normal(0, 0, -1);
    m_avBillBoard[0] = D3DVERTEX(D3DVECTOR(-0.5f, 0.5f, 0), normal, 0, 0);
    m_avBillBoard[1] = D3DVERTEX(D3DVECTOR(-0.5f,-0.5f, 0), normal, 0, 1);
    m_avBillBoard[2] = D3DVERTEX(D3DVECTOR( 0.5f, 0.5f, 0), normal, 1, 0);
    m_avBillBoard[3] = D3DVERTEX(D3DVECTOR( 0.5f,-0.5f, 0), normal, 1, 1);
}

CSelectSrv::~CSelectSrv()
{
    for (int i = 0; i < m_nSrvCount; ++i) delete m_ppxSrvBtn[i];
    delete[] m_ppxSrvBtn;
}

VOID CSelectSrv::Create(CWHWilImageData* image, CPDLList<ServerList>* servers)
{
    for (int i = 0; i < m_nSrvCount; ++i) delete m_ppxSrvBtn[i];
    delete[] m_ppxSrvBtn;
    m_nSrvCount = servers->GetCounter();
    m_ppxSrvBtn = new _PSERVERBUTTON[m_nSrvCount];
    m_pxImage = image;
    m_pressed = m_hover = -1;
    HDC dc = GetDC(g_xMainWnd.GetSafehWnd());
    HFONT font = g_xMainWnd.CreateGameFont("Arial", 12);
    HGDIOBJ previous = SelectObject(dc, font);
    int width = 0, height = 0;
    servers->MoveCurrentToTop();
    for (int i = 0; i < m_nSrvCount; ++i) {
        m_ppxSrvBtn[i] = new _SERVERBUTTON{};
        strcpy(m_ppxSrvBtn[i]->szName, servers->GetCurrentData()->Name);
        strcpy(m_ppxSrvBtn[i]->szCaption, servers->GetCurrentData()->Caption);
        SIZE size = {};
        GetTextExtentPoint32(dc, m_ppxSrvBtn[i]->szCaption, strlen(m_ppxSrvBtn[i]->szCaption), &size);
        width = max(width, size.cx); height = max(height, size.cy);
        servers->MoveNextNode();
    }
    SelectObject(dc, previous); DeleteObject(font); ReleaseDC(g_xMainWnd.GetSafehWnd(), dc);
    const int rowHeight = height + 20;
    SetRect(&m_rcWnd, 20, 60, 20 + width + 90, 80 + rowHeight * m_nSrvCount);
    for (int i = 0; i < m_nSrvCount; ++i)
        SetRect(&m_ppxSrvBtn[i]->rcBtn, 20, 70 + i * rowHeight, m_rcWnd.right, 70 + (i + 1) * rowHeight);
}

INT CSelectSrv::Hit(INT x, INT y) const
{
    POINT point = {x, y};
    for (int i = 0; i < m_nSrvCount; ++i) if (PtInRect(&m_ppxSrvBtn[i]->rcBtn, point)) return i;
    return -1;
}

HRESULT CSelectSrv::OnKeyDown(WPARAM, LPARAM) { return 0; }
HRESULT CSelectSrv::OnButtonDown(WPARAM, LPARAM position)
{
    m_pressed = Hit(LOWORD(position), HIWORD(position)); return 0;
}
HRESULT CSelectSrv::OnButtonDown(POINT point) { return OnButtonDown(0, MAKELPARAM(point.x, point.y)); }
HRESULT CSelectSrv::OnButtonUp(WPARAM, LPARAM position)
{
    const int pressed = m_pressed; m_pressed = -1;
    if (pressed >= 0 && pressed == Hit(LOWORD(position), HIWORD(position)))
        g_xClientSocket.OnSelectServer(m_ppxSrvBtn[pressed]->szName);
    return 0;
}
HRESULT CSelectSrv::OnButtonUp(POINT point) { return OnButtonUp(0, MAKELPARAM(point.x, point.y)); }
LRESULT CSelectSrv::OnMouseMove(WPARAM, LPARAM position)
{
    m_hover = Hit(LOWORD(position), HIWORD(position)); return 0;
}

VOID CSelectSrv::Render(INT)
{
    if (!m_fIsActive) return;
    Draw3DBorder(m_rcWnd, RGB(100,100,100), 120);
    HFONT font = g_xMainWnd.CreateGameFont("Arial", 12);
    for (int i = 0; i < m_nSrvCount; ++i) {
        const bool pressed = m_pressed == i;
        const bool hover = m_pressed < 0 && m_hover == i;
        RECT bounds = m_ppxSrvBtn[i]->rcBtn;
        if (pressed || hover) Draw3DBorder(bounds, pressed ? RGB(255,100,100) : RGB(100,100,100), 50);
        g_xMainWnd.PutsHan(NULL, bounds.left + 20, bounds.top + 10,
            pressed ? RGB(255,100,100) : (hover ? RGB(0,0,255) : RGB(30,30,30)), RGB(0,0,0), m_ppxSrvBtn[i]->szCaption, font);
    }
    DeleteObject(font);
}

VOID CSelectSrv::Draw3DBorder(RECT rcWnd,COLORREF dwColor,INT nAlpha)
{
	if ( g_xMainWnd.Get3DDevice() )
	{
		D3DVECTOR vTrans;
		D3DMATRIX matTrans;
		D3DMATRIX matScale;
		D3DMATRIX matWorld;
		D3DMATRIX matView;
		D3DMATRIX matProj;
		D3DMATRIX matWorldOriginal;
		D3DMATERIAL7 mtrl;
		if( SUCCEEDED(g_xMainWnd.Get3DDevice()->BeginScene()))
		{
			g_xMainWnd.Get3DDevice()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			ZeroMemory(&matView, sizeof(D3DMATRIX));
			D3DVECTOR vEyePt    = D3DVECTOR(0, 0, -(float)(g_xMainWnd.m_stDisplayInfo.wHeight/2));
			D3DVECTOR vLookatPt = D3DVECTOR(0, 0, 0);
			D3DVECTOR vUpVec    = D3DVECTOR(0, 1, 0);

			D3DUtil_SetViewMatrix(matView, vEyePt, vLookatPt, vUpVec);		
			D3DUtil_SetProjectionMatrix(matProj, g_PI/2, float(240.0f/320.0f), -1.0f, 1.0f);

			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_VIEW,       &matView);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

			vTrans.x = (FLOAT)rcWnd.left - 320.0f + (rcWnd.right  - rcWnd.left)/2;
			vTrans.y = (FLOAT)-rcWnd.top + 240.0f - (rcWnd.bottom - rcWnd.top )/2;
			vTrans.z = 0;

			D3DUtil_SetTranslateMatrix(matTrans, vTrans);
			D3DUtil_SetScaleMatrix(matScale, (FLOAT)(rcWnd.right - rcWnd.left), (FLOAT)(rcWnd.bottom - rcWnd.top), 0.0f);
			D3DMath_MatrixMultiply(matWorld, matScale, matTrans);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);

			// ÁÙÊ±
			D3DUtil_InitMaterial(mtrl, 
								 (FLOAT)GetRValue(dwColor)/255.0f,
								 (FLOAT)GetGValue(dwColor)/255.0f, 
								 (FLOAT)GetBValue(dwColor)/255.0f);
			mtrl.diffuse.a = nAlpha/255.0f;					// alpha value

			g_xMainWnd.Get3DDevice()->SetMaterial(&mtrl);

			g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			g_xMainWnd.Get3DDevice()->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
			g_xMainWnd.Get3DDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

			g_xMainWnd.Get3DDevice()->SetTexture(0, NULL);
			g_xMainWnd.Get3DDevice()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, m_avBillBoard, 4, NULL);

			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_ONE);
			g_xMainWnd.Get3DDevice()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);
			g_xMainWnd.Get3DDevice()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorldOriginal);

			g_xMainWnd.Get3DDevice()->EndScene();
		}
	}
}