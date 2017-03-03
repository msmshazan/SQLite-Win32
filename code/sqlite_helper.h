#if !defined(SQLITE_HELPER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Mohamed Shazan $
   $Notice: All Rights Reserved. $
   ======================================================================== */
//*******************************************************
//   Sqlite3 helper funtions
//   TODO: Provide async support
//
//*******************************************************
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

sqlite3* CreateDatabase(const char* FileName){
    sqlite3* Result;
    sqlite3_open(FileName,&Result);
    if(!Result){
        //TODO: Error Handling
        // New database can't only be empty
    }
    return Result;
}

sqlite3* OpenDatabase(const char* FileName){
    sqlite3* Result;
    sqlite3_open(FileName,&Result);
    if(!Result){
        //TODO: Error Handling
        // New database can't only be empty
    }
    return Result;
}

int CloseDatabase(sqlite3* Database){
    int Result = sqlite3_close(Database);
    if(Result){
        //TODO: Error Handling
        // SQLITE_OK == 0
    }
    return Result;
}

// Statements should include Primary key
// and other required fields.
// Example:
//const char create_sql[] = "CREATE TABLE test ("
//      "id INTEGER PRIMARY KEY,"
//      "foo REAL,"
//      "bar TEXT"
//      ")";

int CreateTable(const char* Statement,sqlite3* Database){

    // SQLite exec returns errors with this
    char *exec_errmsg;

    // Use exec to run simple statements that can only fail/succeed
    int Result = sqlite3_exec(Database, Statement, NULL, NULL, &exec_errmsg);
    if(SQLITE_OK != Result) {
        // TODO: Correct error handling
        //fprintf(stderr, "Error creating table (%i): %s\n", Result, exec_errmsg);
        
    }
    return Result;
}


/* Note the question marks. No quoting or anything,
   just put a ? where you'd otherwise put the contents of
   mysql_real_escape_string */
//const char select_sql[] = "SELECT foo, bar FROM test WHERE id=?";
int SelectData(const char* Statement,sqlite3* Database){

    sqlite3_stmt *select_stmt;
    int Result= sqlite3_prepare_v2(Database, Statement, -1, &select_stmt, NULL);
    if(SQLITE_OK != Result) {
        //fprintf(stderr, "Can't prepare select statment %s (%i): %s\n", Statement, Result, sqlite3_errmsg(Database));
        
    }
    // Actually do the select!
    Result = sqlite3_step(select_stmt);
    if(SQLITE_DONE != Result) {
        fprintf(stderr, "select statement didn't return DONE (%i): %s\n", Result, sqlite3_errmsg(Database));
    } else {
        fprintf(stdout,"SELECT completed\n\n");
    }
    sqlite3_finalize(select_stmt);

    return Result;
}

int RetriveData(const char* Statement,sqlite3* Database){

    sqlite3_stmt *retrive_stmt;
    int Result= sqlite3_prepare_v2(Database, Statement, -1, &retrive_stmt, NULL);
    if(SQLITE_OK != Result) {
        //fprintf(stderr, "Can't prepare select statment %s (%i): %s\n", Statement, Result, sqlite3_errmsg(Database));
        
    }
    // Actually do the select!
    Result = sqlite3_step(retrive_stmt);
    if(SQLITE_DONE != Result) {
        fprintf(stderr, "select statement didn't return DONE (%i): %s\n", Result, sqlite3_errmsg(Database));
    } else {
        fprintf(stdout,"SELECT completed\n\n");
        
      
        for(int i=0;i<sqlite3_column_count(retrive_stmt),i++){
            
            printf("%s.\n",sqlite3_column_text(retrive_stmt, i));
    }
        printf("");
    }
    
    sqlite3_finalize(select_stmt);

    return Result;
}
/* Note the question marks. No quoting or anything,
   just put a ? where you'd otherwise put the contents of
   mysql_real_escape_string */
// const char insert_sql[] = "INSERT INTO test (foo, bar) VALUES (?,?)";

int InsertData(const char* Statement,sqlite3* Database){
    
    sqlite3_stmt *insert_stmt;
    int Result= sqlite3_prepare_v2(Database, Statement, -1, &insert_stmt, NULL);
    if(SQLITE_OK != Result) {
        //fprintf(stderr, "Can't prepare insert statment %s (%i): %s\n", Statement, Result, sqlite3_errmsg(Database));
        //sqlite3_close(Database);
        //
    }
    
    // Actually do the insert!
    Result = sqlite3_step(insert_stmt);
    if(SQLITE_DONE != Result) {
        fprintf(stderr, "insert statement didn't return DONE (%i): %s\n", Result, sqlite3_errmsg(Database));
    } else {
        fprintf(stdout,"INSERT completed\n\n");
    }

    sqlite3_finalize(insert_stmt);

    return Result;
}

void ExecuteMultipleQueries(const char* Statements,sqlite3* Database){
    
    // Prepared statment
    sqlite3_stmt *stmt = NULL;

    // This one's the exciting bit of this example
    const char *Tail;

    
    // Seed it since we're just going to keep checking Tail for simplicity
    Tail = (const char *)Statements;

    /* Note that we check strlen because sqlite is effectively returning
       end_of_previous_stmt+1, which is a valid ptr but might be an empty string
    */
    while(Tail && strlen(Tail)) {
        // SQLite return value
        int Result;
        printf("Tail: \"%s\"\n", Tail);

        // Temporarily hold this until the end of the loop in case we need to report an error
        const char *newTail;

        // Prepare the statement, capture the new Tail
        Result = sqlite3_prepare_v2(Database, Tail, -1, &stmt, &newTail);
        if(SQLITE_OK != Result) {
            fprintf(stderr, "Can't prepare statment (%i): %s\n\"%s\"\n", Result, sqlite3_errmsg(Database), Tail);

            
            // Or do something graceful depending on your mood. Best bet is probably:
            Tail = newTail;
            
        }else{
        
            // Step the new statement
            Result = sqlite3_step(stmt);
            if(SQLITE_ROW == Result) {
                fprintf(stderr, "Warning; Query returns rows, which this example ignores.\n\"%s\"\n", Tail);
            } else if(SQLITE_MISUSE == Result) {
                fprintf(stderr, "Got SQLITE_MISUSE.\n"
                        "This is usually the result of the last statement being empty [eg a blank newline]\n\n");
            } else if(SQLITE_DONE != Result && SQLITE_OK != Result) {
                fprintf(stderr, "Error stepping (%i): %s\n\"%s\"\n", Result, sqlite3_errmsg(Database), Tail);
            } else {
                printf("Successfully stepped another query out of Tail\n\n");
            }

            // For next time around the loop
            Tail = newTail;

            // Clear up since we're about to prepare another
            sqlite3_finalize(stmt);
        }
    }


}

int InsertBlob(sqlite3_blob *Blob,sqlite3* Database){
    int Result = 0;
    // TODO: IMPLEMENT
    return Result;
}

#define SQLITE_HELPER_H
#endif
