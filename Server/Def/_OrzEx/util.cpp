

#include "stdafx.h"
#include "util.h"


int _hextodec( char *hex, int len )
{
	int dec  = 0;
	int base = 1;
	int index;

	for ( int i = len - 1; i >= 0; i-- )
	{
		if ( hex[i] >= '0' && hex[i] <= '9' )
			index = hex[i] - '0';
		else
			index = hex[i] - 'A' + 10;

		dec += index * base;
		base *= 16;
	}

	return dec;
}


// 将十六进制整数转换为十六进制字符。
static char _hexdectochar( char num )
{
	if ( num < 10 )
		return num + '0';
	
	return (num - 10) + 'A';
}


void _dectohex( int dec, char *hex, int len )
{
	int index = 0;

	while ( index < len )
	{
		hex[index++] = _hexdectochar( dec % 16 );
		dec = dec / 16;

		if ( dec == 1 )
		{
			if ( index < len )
				hex[index++] = _hexdectochar( dec );
			break;
		}
		else if ( dec == 0 )
		{
			if ( index < len )
				hex[index++] = '0';
			break;
		}
	}

	// 将倒序保存的十六进制字符串反转。
	for ( int i = 0; i < index / 2; i++ )
		_swap( hex[i], hex[index - i - 1] );
}
