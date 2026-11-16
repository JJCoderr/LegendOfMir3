

/*
	Prime Class

	Date:
		2001/02/05 (Last Updated: 2002/03/06)

	Note:
		与素数(Prime Number)相关的函数集合
*/
#ifndef __ORZ_ALGORITHM_PRIME__
#define __ORZ_ALGORITHM_PRIME__


class CPrime
{
public:
	static bool IsPrime( int nNumber );
	static int  RoundUp( int nNumber );
	static int  RoundDown( int nNumber );
};


#endif