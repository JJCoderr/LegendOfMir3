#include "Common/LoginMessages.h"
#include "../SelectionRoster.h"
#include "../RoleAnimation.h"
// LoginProcess.h: interface for the CSelectChrProcess class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SELECT_CHR_H
#define _SELECT_CHR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSelectChr : public CBMWnd 
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CSelectChr();
	~CSelectChr();

	virtual VOID		Create(CWHWilImageData* pxImage,CPDLList<SelectionRoleInfo>* pxList, CPDLList<CHRRECT>* pxrcList,INT nChrCount);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonUp(POINT ptMouse);
	virtual HRESULT		OnKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseMove(WPARAM wParam, LPARAM lParam);
	virtual VOID		OpenWnd(VOID);
	virtual VOID		CloseWnd(VOID);

	VOID				ShowChrInfo(VOID);
	virtual	VOID		Render(INT	nLoopTime);
private:
    friend struct SelectionRosterPreview;
    friend struct CharacterInteractionPreview;
    friend struct RolePresentationPreview;
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
	INT					m_nRenderState;
	INT					m_nSelectChr;
private:
protected:
    POINT m_pointer={-1,-1};
    RoleAnimation m_roleAnimation[SelectionRoster::Capacity];
    int m_page = 0, m_pagePressed = -1;
    CWHWilImageData m_pageImages;
    int PageHit(LPARAM position);
	CLoginMessages m_messages;
	CLightFog			m_xLightFog;
	CPDLList<SelectionRoleInfo>*	m_pxSList;
	CPDLList<CHRRECT>*	m_pxrcList;
	CWHWilImageData		*m_pxImage;
	CMirButton			**m_pBtn;
	POINT				m_PosName;
	BOOL				m_fIsButtonDown;
	INT					m_nChrCount;
	INT					m_nAnimationCount;
	INT					m_nBlend;
	INT					m_nRoll;
};
#endif //_SELECT_CHR_H
