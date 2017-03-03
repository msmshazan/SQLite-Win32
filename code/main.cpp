/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Mohamed Shazan $
   $Notice: All Rights Reserved. $
   ======================================================================== */

#include "sqlite_helper.h"
//---Testing---
int main(){
    
    sqlite3* Database = CreateDatabase("Test.db");
    int Result = CreateTable("CREATE TABLE test (id INTEGER PRIMARY KEY,test1 REAL,test2 TEXT)",Database);
    int Result_select = SelectData("SELECT test1,test2 FROM test WHERE id=?",Database);
    int Result_insert = InsertData("INSERT INTO test (test1,test2) VALUES (?,?)",Database);
    int Result_select = SelectData("SELECT test1,test2 FROM test WHERE id=?",Database);
    #if 0
    ExecuteMultipleQueries("CREATE TABLE test(id INTEGER PRIMARY KEY,test1 REAL,test2 TEXT);\n"
                           "SELECT test1,test2 FROM test WHERE id=1;\n"
                           "INSERT INTO test (test1,test2) VALUES (0,1);",Database);
    #endif
    return 0;
}
