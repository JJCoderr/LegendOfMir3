#include "../Def/_OrzEx/database.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static void Check(bool ok, const char* step)
{
    if (!ok) { fprintf(stderr, "FAIL: %s\n", step); exit(1); }
}
static bool Execute(CRecordset* record, const char* sql)
{
    return record->Execute(const_cast<char*>(sql));
}
int main()
{
    CDatabase db;
    Check(db.Init(), "database init");
    CConnection* connection = db.CreateConnection(const_cast<char*>("sqlite::memory:"), NULL, NULL);
    Check(connection != NULL, "SQLite connection");
    CRecordset* record = connection->CreateRecordset();
    Check(record != NULL, "recordset");
    Check(Execute(record, "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT UNIQUE, optional TEXT)"), "schema");
    Check(Execute(record, "INSERT INTO test VALUES (1, 'first', NULL)"), "insert");
    Check(record->GetRowCount() == 1, "affected rows");
    Check(!Execute(record, "INSERT INTO test VALUES (1, 'duplicate', NULL)"), "constraint failure");
    Check(Execute(record, "SELECT name, optional FROM test"), "select");
    Check(record->Fetch(), "fetch");
    Check(strcmp(record->Get(const_cast<char*>("NAME")), "first") == 0, "case insensitive columns");
    Check(strcmp(record->Get(1), "") == 0, "null text compatibility");
    Check(!record->Fetch() && !record->Fetch(), "end of rows");
    Check(Execute(record, "BEGIN"), "begin");
    Check(Execute(record, "UPDATE test SET name='rolled back'"), "update");
    Check(record->GetRowCount() == 1, "update count");
    Check(Execute(record, "ROLLBACK"), "rollback");
    Check(Execute(record, "SELECT name FROM test") && record->Fetch(), "select after rollback");
    Check(strcmp(record->Get(0), "first") == 0, "rollback value");
    Check(!Execute(record, "DELETE FROM test; DELETE FROM test;"), "reject multiple statements");
    char chinese[64] = {};
    WideCharToMultiByte(CP_ACP, 0, L"\x4f20\x5947", -1, chinese, sizeof(chinese), NULL, NULL);
    std::string query = "UPDATE test SET name='" + std::string(chinese) + "'";
    Check(Execute(record, query.c_str()), "Chinese update");
    Check(Execute(record, "SELECT name FROM test") && record->Fetch(), "Chinese select");
    Check(strcmp(record->Get(0), chinese) == 0, "Chinese roundtrip");
    std::string large(2048, 'x');
    query = "UPDATE test SET name='" + large + "'";
    Check(Execute(record, query.c_str()), "long text update");
    Check(Execute(record, "SELECT name FROM test") && record->Fetch(), "long text select");
    Check(strlen(record->Get(0)) == large.size(), "no text truncation");
    connection->DestroyRecordset(record);
    db.DestroyConnection(connection);
    char temporaryDirectory[MAX_PATH] = {};
    char temporaryFile[MAX_PATH] = {};
    Check(GetTempPathA(MAX_PATH, temporaryDirectory) != 0, "temporary directory");
    Check(GetTempFileNameA(temporaryDirectory, "m3s", 0, temporaryFile) != 0, "temporary database");
    std::string fileConnection = "sqlite:" + std::string(temporaryFile);
    connection = db.CreateConnection(const_cast<char*>(fileConnection.c_str()), NULL, NULL);
    Check(connection != NULL, "file connection");
    record = connection->CreateRecordset();
    Check(record != NULL && Execute(record, "CREATE TABLE persisted (value TEXT)"), "file schema");
    Check(Execute(record, "INSERT INTO persisted VALUES ('saved')"), "file write");
    connection->DestroyRecordset(record);
    db.DestroyConnection(connection);
    connection = db.CreateConnection(const_cast<char*>(fileConnection.c_str()), NULL, NULL);
    Check(connection != NULL, "reopen file");
    record = connection->CreateRecordset();
    Check(record != NULL && Execute(record, "SELECT value FROM persisted") && record->Fetch(), "read persisted row");
    Check(strcmp(record->Get(0), "saved") == 0, "persisted value");
    connection->DestroyRecordset(record);
    db.DestroyConnection(connection);
    Check(DeleteFileA(temporaryFile) != 0, "close and remove temporary database");
    db.Uninit();
    puts("SQLite backend smoke checks passed");
    return 0;
}
