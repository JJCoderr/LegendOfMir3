

/*
	Date:
		2002/03/06

	Note:
		Modified Version for Precompiler and Unicode
*/
#ifndef __ORZ_DATABASE__
#define __ORZ_DATABASE__


#include <windows.h>
#include <sql.h>
#include <sqlext.h>
struct sqlite3;
struct sqlite3_stmt;


#define DB_MAXBUF	256


class CConnection;
class CRecordset;


/*
	define CDatabase 
*/
class CDatabase
{
protected:
	SQLHENV		m_hEnv;

public:
	CDatabase();
	virtual ~CDatabase();

	bool Init();
	void Uninit();

	void EnumDSN( void (*pfnEnum)( char *pSrcName, char *pSrcDesc ) );

	CConnection * CreateConnection( char *pDSN, char *pID, char *pPassword );
	void DestroyConnection( CConnection *pConn );

public:
	static void SetDiagRec( void (*pfnRecord)( char *pState, int nErrCode, char *pDesc ) );
	static void UnsetDiagRec();
	static void SQLiteError(sqlite3* db, int code);
	static void DiagRec( int nHandleType, SQLHANDLE hHandle );
};


/*
	define CConnection 
*/
class CConnection
{
protected:
	SQLHDBC		m_hDBConn;
	sqlite3* m_sqlite;
	bool InitSQLite(const char* path);

private:
	CConnection();
	virtual ~CConnection();

	bool Init( SQLHENV hEnv, char *pDSN, char *pID, char *pPassword );
	void Uninit();

public:
	friend CDatabase;
    bool BeginTransaction();
    bool EndTransaction(bool commit);

	CRecordset * CreateRecordset();
	void DestroyRecordset( CRecordset *pRec );
};


/*
	define CRecordset 
*/
class CRecordset
{
protected:
	SQLHSTMT	m_hStmt;
	sqlite3* m_sqlite;
	sqlite3_stmt* m_sqliteStmt;
	bool ExecuteSQLite(char* query);
	bool FetchSQLite();
	void ResetSQLite();

	int			m_nRowCount;
	int			m_nCols;

	class CColumnInfo
	{
	public:
		int  nColNameSize;			// 列 名称 大小
		char szColName[DB_MAXBUF];	// 列名
		int  nColType;				// 列类型
		int  nColSize;				// 列大小
		int  nAllowDecimalDigit;	// 是否允许十进制变更
		int  nAllowNull;			// 是否允许空值

	public:
		CColumnInfo();
		~CColumnInfo();

	} *m_pColInfo;

	class CColumnData
	{
	public:
		char *pData;				// 数据
		int  nDataSize;				// 纯数据大小 (不是已分配的内存大小)

	public:
		CColumnData();
		~CColumnData();

		bool AllocMemory( int nSize );

	} *m_pColData;

private:
	CRecordset();
	virtual ~CRecordset();

	bool Init( SQLHDBC hDBConn );
	void Uninit();

public:
	friend CConnection;
	
	bool Execute( char *pQuery );
	bool Fetch();

	int  GetRowCount();
	int  GetCols();
	char * Get( char *pColName );
	char * Get( int nCol );
	CColumnInfo * GetColInfo( char *pColName );
	CColumnInfo * GetColInfo( int nCol );
};


#endif