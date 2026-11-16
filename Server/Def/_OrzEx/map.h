

/*
	Map

	Date:
		2001/02/05

	Note:
		使用哈希表的地图类
		使用 Division Hash 函数，并对每个桶执行二分查找。
		Key 使用字符串(char *)。

	Usage:
		用 InitHashTable 分配所需内存，用 UninitHashTable 释放内存。
		使用之前必须先调用 SetGetKeyFunction。
*/
#ifndef __ORZ_DATASTRUCTURE_MAP__
#define __ORZ_DATASTRUCTURE_MAP__



#include "bstree.h"
#include "prime.h"


// 初始化哈希表时使用的标志
#define IHT_UNTOUCH		1	// 直接使用指定的大小。
#define IHT_ROUNDUP		2	// 使用与指定大小最接近的质数(较大的值).
#define IHT_ROUNDDOWN	3	// 使用与指定大小最接近的较小值.


/*
	CMap 类定义

	Note: 使用 T 的指针.
*/
template< class T >
class CMap
{
protected:
	CBsTree< T > *m_pHashTable;

	int  m_nDemandSize;
	int  m_nRealSize;
	int  m_nCount;

	char * (*m_pfnGetKey)( T *pData ); 

public:
	CMap();
	virtual ~CMap();

	virtual bool InitHashTable( int nDemandSize, int nFlags = IHT_UNTOUCH );
	virtual void UninitHashTable( bool bDeleteData = true, bool bDeleteArray = false );

	virtual void SetGetKeyFunction( char * (*pfnGetKey)( T * ) );

	virtual bool Insert( T *pData );
	virtual T *  Remove( T *pKey );
	virtual T *  Search( T *pKey );
	virtual T *  SearchKey( char *pKey );
	
	virtual int  GetCount();
	virtual bool IsEmpty();

	virtual int  GetHashKey( T *pData, bool bUseCallbackFunc = true );

protected:
	static int __cbCmpString( void *pArg, T *pFirst, T *pSecond );
	static int __cbCmpKeyString( void *pArg, T *pData, char *pKey );
};




/*
	实现
*/
template< class T >
CMap< T >::CMap()
{
	m_pHashTable	= NULL;
	m_nDemandSize	= 0;
	m_nRealSize		= 0;
	m_nCount		= 0;
	m_pfnGetKey		= NULL;
}


template< class T >
CMap< T >::~CMap()
{
}


template< class T >
bool CMap< T >::InitHashTable( int nDemandSize, int nFlags )
{
	m_nDemandSize = nDemandSize;
	
	// 求出实际的桶大小.
	switch ( nFlags )
	{
	case IHT_UNTOUCH:
		m_nRealSize = nFlags;
		break;

	case IHT_ROUNDUP:
		m_nRealSize = CPrime::RoundUp( nDemandSize );
		break;

	case IHT_ROUNDDOWN:
		m_nRealSize = CPrime::RoundDown( nDemandSize );
		break;
	}

	// 节点数组分配
	m_pHashTable = new CBsTree< T >[ m_nRealSize ];
	if ( m_pHashTable )
	{
		for ( int i = 0; i < m_nRealSize; i++ )
		{
			m_pHashTable[i].SetCompareFunction( __cbCmpString, this );
			m_pHashTable[i].SetCompareStringFunction( __cbCmpKeyString, this );
		}
	}

	return m_pHashTable ? true : false;
}


template< class T >
void CMap< T >::UninitHashTable( bool bDeleteData, bool bDeleteArray )
{
	if ( m_pHashTable )
	{
		for ( int i = 0; i < m_nRealSize; i++ )
			m_pHashTable[i].ClearAll( bDeleteData, bDeleteArray );

		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}
}



template< class T >
void CMap< T >::SetGetKeyFunction( char * (*pfnGetKey)( T * ) )
{
	m_pfnGetKey = pfnGetKey;
}


template< class T >
bool CMap< T >::Insert( T *pData )
{
	if ( m_pHashTable[ GetHashKey( pData ) ].Insert( pData ) )
	{
		++m_nCount;
		return true;
	}

	return false;
}



template< class T >
T * CMap< T >::Remove( T *pKey )
{
	T *pData = m_pHashTable[ GetHashKey( pKey ) ].Remove( pKey );
	if ( pData )
		--m_nCount;

	return pData;
}


template< class T >
T * CMap< T >::Search( T *pKey )
{
	return m_pHashTable[ GetHashKey( pKey ) ].Search( pKey );
}


template< class T >
T * CMap< T >::SearchKey( char *pKey )
{
	return m_pHashTable[ GetHashKey( (T *) pKey, false ) ].SearchKeyString( pKey );
}


template< class T >
int CMap< T >::GetCount()
{
	return m_nCount;
}


template< class T >
bool CMap< T >::IsEmpty()
{
	return m_nCount == 0;
}


template< class T >
int CMap< T >::GetHashKey( T *pData, bool bUseCallbackFunc )
{
	int  nKeyLen = 0;
	char *pKey = bUseCallbackFunc ? m_pfnGetKey( pData ) : (char *) pData;

	while ( *pKey )
		nKeyLen += (unsigned char) *pKey++;

	return nKeyLen % m_nRealSize;
}


template< class T >
int CMap< T >::__cbCmpString( void *pArg, T *pFirst, T *pSecond )
{
	return strcmp( ((CMap< T > *) pArg)->m_pfnGetKey( pFirst ), ((CMap< T > *) pArg)->m_pfnGetKey( pSecond ) );
}


template< class T >
int CMap< T >::__cbCmpKeyString( void *pArg, T *pData, char *pString )
{
	return strcmp( ((CMap< T > *) pArg)->m_pfnGetKey( pData ), pString );
}


#endif


