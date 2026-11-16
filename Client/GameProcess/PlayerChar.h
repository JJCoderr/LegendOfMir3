/******************************************************************************************************************

	模块名:

	作者:
	编写日期:

	[日期][修改人] : 修改内容

*******************************************************************************************************************/


#ifndef _CPLAYERCHAR_H
#define _CPLAYERCHAR_H


#include <ddraw.h>
#include <mmsystem.h>


class CPlayerChar : public CCharacter
{
private:	
public:
	CDLList<CItem>		m_xBeltItemList;

protected:
public:
	CPlayerChar();
	~CPlayerChar();
};

#endif // _CPLAYERCHAR_H