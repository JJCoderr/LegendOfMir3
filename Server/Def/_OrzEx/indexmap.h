

/*
	IndexMap

	Date:
		2001/02/05

	Note:
		为了提升遍历(Traverse)速度, 内部使用 List.
		当然, 由于使用索引, 插入/删除时需要更多的 CPU Time 和内存.
		不对列表的 Key 进行排序.
*/
#ifndef __ORZ_DATASTRUCTURE_INDEX_MAP__
#define __ORZ_DATASTRUCTURE_INDEX_MAP__


#include "list.h"
#include "map.h"


template< class T >
class CIndexMap : public CMap< T >
{
protected:
	CList< T > m_Index;

public:
	CIndexMap();
	virtual ~CIndexMap();

	virtual bool InitHashTable( int nDemandSize, int nFlags = IHT_UNTOUCH );
	virtual void UninitHashTable( bool bDeleteData = true, bool bDeleteArray = false );

	bool Insert( T *pData );
	T *  Remove( T *pKey );

	CListNode< T > * GetHead();
	CListNode< T > * GetNext( CListNode< T > *pNode );
};


template< class T >
CIndexMap< T >::CIndexMap()
{
}


template< class T >
CIndexMap< T >::~CIndexMap()
{
}


template< class T >
bool CIndexMap< T >::InitHashTable( int nDemandSize, int nFlags )
{
	m_Index.SetCompareFunction( __cbCmpString, this );

	return CMap< T >::InitHashTable( nDemandSize, nFlags );
}


template< class T >
void CIndexMap< T >::UninitHashTable( bool bDeleteData, bool bDeleteArray )
{
	m_Index.ClearAll( false );

	CMap< T >::UninitHashTable( bDeleteData, bDeleteArray );
}


template< class T >
bool CIndexMap< T >::Insert( T *pData )
{
	if ( CMap< T >::Insert( pData ) )
		return m_Index.Insert( pData );

	return false;
}


template< class T >
T * CIndexMap< T >::Remove( T *pKey )
{
	if ( CMap< T >::Remove( pKey ) )
		return m_Index.Remove( pKey );

	return NULL;
}


template< class T >
CListNode< T > * CIndexMap< T >::GetHead()
{
	return m_Index.GetHead();
}


template< class T >
CListNode< T > * CIndexMap< T >::GetNext( CListNode< T > *pNode )
{
	return m_Index.GetNext( pNode );
}


#endif
