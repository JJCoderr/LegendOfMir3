/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#ifndef _C3DINIT_H
#define _C3DINIT_H


class C3DInit
{
//1: Constructor/Destructor
public:
	C3DInit();
	~C3DInit();

private:
	LPDIRECT3DDEVICE7		m_pd3dDevice;

protected:
	D3DMATRIX				m_matWorld;
	D3DMATRIX				m_matView;
	D3DMATRIX				m_matProj;
	D3DMATERIAL7			m_mtrl;

public:
	HRESULT InitDeviceObjects();
                                      
	__inline static C3DInit* GetObj()			{  return m_px3DInit;  }
	__inline static VOID SetObj(C3DInit* pxObj)	{  m_px3DInit = pxObj; }
protected:
	static C3DInit* m_px3DInit;
};


#endif //_C3DINIT_H