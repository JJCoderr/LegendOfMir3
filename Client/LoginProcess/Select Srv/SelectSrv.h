// LoginProcess.h: interface for the CLoginProcess class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SELECT_SERVER_H
#define _SELECT_SERVER_H
#include "Common/LoginMessages.h"
#include <vector>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSelectSrv : public CBMWnd 
{
	/////////////////////////////////////////////////////////////////////////////////////////
	// Functions
public:
	CSelectSrv();
    bool ApplyLoads(const char* body);
    void ResetPointer() {m_pressed=m_hover=-1;}
	~CSelectSrv();

	virtual VOID		Create(CWHWilImageData* pxImage,CPDLList<ServerList>* pxSList);

	virtual HRESULT		OnButtonDown(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonDown(POINT ptMouse);
	virtual HRESULT		OnButtonUp(WPARAM wParam, LPARAM lParam);
	virtual HRESULT		OnButtonUp(POINT ptMouse);
	virtual HRESULT		OnKeyDown(WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseMove(WPARAM wParam, LPARAM lParam);

	virtual	VOID		Render(INT	nLoopTime);
private:
protected:
	/////////////////////////////////////////////////////////////////////////////////////////
	// Variable
public:
private:
	friend struct LoginPreview;
    friend class CLoginProcess;
    std::vector<int> m_loads;
    CLoginMessages m_messages;
    INT m_pressed, m_hover;
	INT Hit(INT x, INT y) const;
protected:
	_PPSERVERBUTTON			m_ppxSrvBtn;
	INT						m_nSrvCount;
};
#endif //_SELECT_SERVER_H
