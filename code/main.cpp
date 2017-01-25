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
    return 0;
}
