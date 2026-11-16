#include "stdafx.h"
#include "database.h"
#include "../sqlite/sqlite3.h"
#include <string>
#include <cstring>
#include <cctype>

static std::string ConvertDatabaseText(const char* text, UINT from, UINT to)
{
    if (!text || !*text) return std::string();
    int count = MultiByteToWideChar(from, 0, text, -1, NULL, 0);
    std::wstring wide(count, L'\0');
    if (!count || !MultiByteToWideChar(from, 0, text, -1, &wide[0], count))
        return std::string();
    count = WideCharToMultiByte(to, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(count, '\0');
    if (!count || !WideCharToMultiByte(to, 0, wide.c_str(), -1, &result[0], count, NULL, NULL))
        return std::string();
    result.resize(count - 1);
    return result;
}

bool CConnection::InitSQLite(const char* path)
{
    if (!path || !*path) return false;
    std::string utf8 = ConvertDatabaseText(path, CP_ACP, CP_UTF8);
    int result = sqlite3_open_v2(utf8.c_str(), &m_sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (result == SQLITE_OK) {
        sqlite3_busy_timeout(m_sqlite, 5000);
        result = sqlite3_exec(m_sqlite,
            "PRAGMA foreign_keys=ON; PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    }
    if (result == SQLITE_OK) return true;
    CDatabase::SQLiteError(m_sqlite, result);
    sqlite3_close(m_sqlite);
    m_sqlite = NULL;
    return false;
}

void CRecordset::ResetSQLite()
{
    if (m_sqliteStmt) sqlite3_finalize(m_sqliteStmt);
    m_sqliteStmt = NULL;
    delete[] m_pColInfo;
    delete[] m_pColData;
    m_pColInfo = NULL;
    m_pColData = NULL;
    m_nCols = 0;
    m_nRowCount = 0;
}

bool CRecordset::ExecuteSQLite(char* query)
{
    ResetSQLite();
    if (!query) return false;
    std::string sql = ConvertDatabaseText(query, CP_ACP, CP_UTF8);
    const char* tail = NULL;
    int result = sqlite3_prepare_v2(m_sqlite, sql.c_str(), -1, &m_sqliteStmt, &tail);
    if (result != SQLITE_OK) {
        CDatabase::SQLiteError(m_sqlite, result);
        ResetSQLite();
        return false;
    }
    // Do not silently discard a second statement or execute half a batch.
    while (tail && *tail && std::isspace(static_cast<unsigned char>(*tail))) ++tail;
    if (!m_sqliteStmt || (tail && *tail)) {
        ResetSQLite();
        return false;
    }
    m_nCols = sqlite3_column_count(m_sqliteStmt);
    if (!m_nCols) {
        sqlite3_int64 before = sqlite3_total_changes64(m_sqlite);
        result = sqlite3_step(m_sqliteStmt);
        if (result != SQLITE_DONE) CDatabase::SQLiteError(m_sqlite, result);
        m_nRowCount = result == SQLITE_DONE && sqlite3_total_changes64(m_sqlite) != before
            ? sqlite3_changes(m_sqlite) : 0;
        sqlite3_finalize(m_sqliteStmt);
        m_sqliteStmt = NULL;
        return result == SQLITE_DONE;
    }
    m_pColInfo = new CColumnInfo[m_nCols];
    m_pColData = new CColumnData[m_nCols];
    for (int i = 0; i < m_nCols; ++i) {
        std::string name = ConvertDatabaseText(sqlite3_column_name(m_sqliteStmt, i), CP_UTF8, CP_ACP);
        if (name.size() >= DB_MAXBUF) {
            ResetSQLite();
            return false;
        }
        strcpy(m_pColInfo[i].szColName, name.c_str());
        m_pColInfo[i].nColNameSize = static_cast<int>(name.size());
        m_pColInfo[i].nAllowNull = 1;
        m_pColData[i].AllocMemory(1);
    }
    return true;
}

bool CRecordset::FetchSQLite()
{
    if (!m_sqliteStmt) return false;
    int result = sqlite3_step(m_sqliteStmt);
    if (result != SQLITE_ROW) {
        if (result != SQLITE_DONE) CDatabase::SQLiteError(m_sqlite, result);
        sqlite3_finalize(m_sqliteStmt);
        m_sqliteStmt = NULL;
        return false;
    }
    for (int i = 0; i < m_nCols; ++i) {
        const char* value = reinterpret_cast<const char*>(sqlite3_column_text(m_sqliteStmt, i));
        std::string text = ConvertDatabaseText(value, CP_UTF8, CP_ACP);
        delete[] m_pColData[i].pData;
        m_pColData[i].pData = NULL;
        m_pColData[i].AllocMemory(static_cast<int>(text.size()) + DB_MAXBUF);
        memcpy(m_pColData[i].pData, text.c_str(), text.size() + 1);
        m_pColData[i].nDataSize = value ? static_cast<int>(text.size()) : SQL_NULL_DATA;
        m_pColInfo[i].nColSize = static_cast<int>(text.size());
    }
    ++m_nRowCount;
    return true;
}
