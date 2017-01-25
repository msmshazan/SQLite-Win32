/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Mohamed Shazan $
   $Notice: All Rights Reserved. $
   ======================================================================== */
#define _CRT_SECURE_NO_WARNINGS
#include "main.h"
#include "sqlite_helper.h"
#if 0
/**
 Example showing usage of sqlite3 API to insert blobs by inserting a
    placeholder then writing to the blob handle

  Note that you could also read the whole thing into memory and do the insert with a bind.
  Doing it by opening the blob handle allows you to do blockwise reading/insertion,
    thereby inserting huge files without much memory consumption
 
*/
int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <dbname> <filename>\n", argv[0]);
        exit(1);
    }

    // file to insert
    FILE *f = fopen(argv[2], "rb");
    if(NULL == f) {
        fprintf(stderr, "Couldn't open file %s\n", argv[2]);
        exit(1);
    }
    // Calculate size of file
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Table name we're going to use
    char tablename[] = "testblob";
    char columnname[] = "blobby";

    // Actual database handle
    sqlite3 *db = NULL;

    // Database commands
    char create_sql[1024];
    snprintf(create_sql, sizeof(create_sql), "CREATE TABLE IF NOT EXISTS %s ("
            "id INTEGER PRIMARY KEY, %s BLOB)", tablename, columnname);

    // Going to insert a zeroblob of the size of the file
    char insert_sql[1024];
    snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO %s (%s) VALUES (?)", tablename, columnname);

    // SQLite return value
    int rc;

    // Open the database
    rc = sqlite3_open(argv[1], &db);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't open database %s (%i): %s\n", argv[1], rc, sqlite3_errmsg(db));
        exit(1);
    }

    char *exec_errmsg;
    rc = sqlite3_exec(db, create_sql, NULL, NULL, &exec_errmsg);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't create table (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    sqlite3_stmt *insert_stmt;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't prepare insert statment %s (%i): %s\n", insert_sql, rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    // Bind a block of zeros the size of the file we're going to insert later
    sqlite3_bind_zeroblob(insert_stmt, 1, filesize);
    if(SQLITE_DONE != (rc = sqlite3_step(insert_stmt))) {
        fprintf(stderr, "Insert statement didn't work (%i): %s\n", rc, sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
    printf("Created a row, id %i, with a blank blob size %i\n", (int)rowid, (int)filesize);

    // Getting here means we have a valid file handle, f, and a valid db handle, db
    //   Also, a blank row has been inserted with key rowid
    sqlite3_blob *blob;
    rc = sqlite3_blob_open(db, "main", tablename, columnname, rowid, 1, &blob);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Couldn't get blob handle (%i): %s\n", rc, sqlite3_errmsg(db));
        exit(1);
    }

    const int BLOCKSIZE = 1024;
    int len;
    void *block = malloc(BLOCKSIZE);
    
    int offset = 0;
    while(0 < (len = fread(block, 1, BLOCKSIZE, f))) {
        if(SQLITE_OK != (rc = sqlite3_blob_write(blob, block, len, offset))) {
            fprintf(stderr, "Error writing to blob handle. Offset %i, len %i\n", offset, len);
            exit(1);
        }
        offset+=len;
    }

    sqlite3_blob_close(blob);

    printf("Successfully wrote to blob\n");

    free(block);

    fclose(f);
    sqlite3_finalize(insert_stmt);
    sqlite3_close(db);
    return 0;
}
#endif

#if 0
/**
 Example showing correct way to execute a whole bunch of SQL queries in one string
*/

int main() {
    // Create an in-memory database
    const char dbname[] = "sqlite.db";

    // Actual database handle
    sqlite3 *db = NULL;

    // Database commands
    //  Note that these are all catenated into one long string
    //  Not also that there are no SELECTs here; I'm
    //     effectively reading a .DUMP or something.
    const char sql[] = "CREATE TABLE test (foo REAL);\n"
            "INSERT INTO test VALUES (3.141);\n"
            "INSERT INTO test VALUES (2.718);\n"
            "INSERT INTO test VALUES (42);\n";

    // Prepared statment
    sqlite3_stmt *stmt = NULL;

    // This one's the exciting bit of this example
    const char *zTail;

    // SQLite return value
    int rc;

    // Open the database
    rc = sqlite3_open(dbname, &db);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't open database %s (%i): %s\n", dbname, rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }


    // Seed it since we're just going to keep checking zTail for simplicity
    zTail = (const char *)sql;

    /* Note that we check strlen because sqlite is effectively returning
         end_of_previous_stmt+1, which is a valid ptr but might be an empty string
    */
    while(zTail != NULL && 0 < strlen(zTail)) {
        printf("zTail: \"%s\"\n", zTail);

        // Temporarily hold this until the end of the loop in case we need to report an error
        const char *newzTail;

        // Prepare the statement, capture the new zTail
        rc = sqlite3_prepare_v2(db, zTail, -1, &stmt, &newzTail);
        if(SQLITE_OK != rc) {
            fprintf(stderr, "Can't prepare statment (%i): %s\n\"%s\"\n", rc, sqlite3_errmsg(db), zTail);
            sqlite3_close(db);
            exit(1);

            /*
            // Or do something graceful depending on your mood. Best bet is probably:
            zTail = newzTail;
            continue;
            */
        }
        
        // Step the new statement
        rc = sqlite3_step(stmt);
        if(SQLITE_ROW == rc) {
            fprintf(stderr, "Warning; Query returns rows, which this example ignores.\n\"%s\"\n", zTail);
        } else if(SQLITE_MISUSE == rc) {
            fprintf(stderr, "Got SQLITE_MISUSE.\n"
                "This is usually the result of the last statement being empty [eg a blank newline]\n\n");
        } else if(SQLITE_DONE != rc && SQLITE_OK != rc) {
            fprintf(stderr, "Error stepping (%i): %s\n\"%s\"\n", rc, sqlite3_errmsg(db), zTail);
        } else {
            printf("Successfully stepped another query out of zTail\n\n");
        }

        // For next time around the loop
        zTail = newzTail;

        // Clear up since we're about to prepare another
        sqlite3_finalize(stmt);
    }

    // And thus ends the code demo

    printf("Finished stepping all statements\n\n"
        "SELECT the contents of the db, just to check it worked:\n");


    // This last bit's just to show it did what you wanted.


    sqlite3_stmt *select_stmt;
    rc = sqlite3_prepare_v2(db, "SELECT * FROM test", -1, &select_stmt, NULL);
    while(SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
        int col;
        printf("Found row\n");
        for(col=0; col<sqlite3_column_count(select_stmt); col++) {
            // Note that by using sqlite3_column_text, sqlite will coerce the value into a string
            printf("\tColumn %s(%i): '%s'\n",
                sqlite3_column_name(select_stmt, col), col,
                sqlite3_column_text(select_stmt, col));
        }
    }
    if(SQLITE_DONE != rc) {
        fprintf(stderr, "select statement didn't finish with DONE (%i): %s\n", rc, sqlite3_errmsg(db));
    } else {
        printf("\nSELECT successfully completed\n");
    }


    sqlite3_finalize(select_stmt);
    sqlite3_close(db);
    return 0;
}

#endif

#if 1
//---Testing---
int main(){

    sqlite3* Database = CreateDatabase("Test.db");
    int Result = CreateTable("CREATE TABLE test (id INTEGER PRIMARY KEY,test1 REAL,test2 TEXT)",Database);
    int Result_select = SelectData("SELECT test1,test2 FROM test WHERE id=?",Database);
    int Result_insert = InsertData("INSERT INTO test (test1,test2) VALUES (?,?)",Database);
    return 0;
}

#endif

#if 0
/**
 Example showing correct usage of sqlite3 API commands bind, prepare, step
 */
int main() {
    // Create an in-memory database
    const char dbname[] = "sqlite.db";

    // Actual database handle
    sqlite3 *db = NULL;

    // Database commands
    const char create_sql[] = "CREATE TABLE test ("
            "id INTEGER PRIMARY KEY,"
            "foo REAL,"
            "bar TEXT"
            ")";

    /* Note the question marks. No quoting or anything,
        just put a ? where you'd otherwise put the contents of
        mysql_real_escape_string */
    const char insert_sql[] = "INSERT INTO test (foo, bar) VALUES (?,?)";
    const char select_sql[] = "SELECT foo, bar FROM test WHERE id=?";

    // For the insert and select, we will prepare statements
    sqlite3_stmt *insert_stmt = NULL;
    sqlite3_stmt *select_stmt = NULL;

    // SQLite return value
    int rc;

    // Open the database
    rc = sqlite3_open(dbname, &db);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't open database %s (%i): %s\n", dbname, rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }


    // SQLite exec returns errors with this
    char *exec_errmsg;

    // Use exec to run simple statements that can only fail/succeed
    rc = sqlite3_exec(db, create_sql, NULL, NULL, &exec_errmsg);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Error creating table (%i): %s\n", rc, exec_errmsg);
        sqlite3_free(exec_errmsg);
        sqlite3_close(db);
        exit(1);
    }



    
    // Prepare the two statements we need to prepare
    //  The last parameter, dbend, is used if you have multiple sql statements in one string
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't prepare insert statment %s (%i): %s\n", insert_sql, rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    rc = sqlite3_prepare_v2(db, select_sql, -1, &select_stmt, NULL);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Can't prepare select statment %s (%i): %s\n", select_sql, rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Now actually bind the values (1-indexed)
    double realvalue = 3.14159;
    rc = sqlite3_bind_double(insert_stmt, 1, realvalue);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in insert (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    } else {
        printf("Successfully bound real for insert: %f\n", realvalue);
    }

    const char injectionattack[] =  "Chunkys'; DROP TABLE test;";
    // The NULL is "Don't attempt to free() the value when it's bound", since it's on the stack here
    rc = sqlite3_bind_text(insert_stmt, 2, injectionattack, sizeof(injectionattack), NULL);
    if(SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in insert (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    } else {
        printf("Successfully bound string for insert: '%s'\n", injectionattack);
    }


    // Actually do the insert!
    rc = sqlite3_step(insert_stmt);
    if(SQLITE_DONE != rc) {
        fprintf(stderr, "insert statement didn't return DONE (%i): %s\n", rc, sqlite3_errmsg(db));
    } else {
        printf("INSERT completed\n\n");
    }


    // Now attempt to get that row out
    sqlite3_int64 id = sqlite3_last_insert_rowid(db);
    sqlite3_bind_int64(select_stmt, 1, id);

    // This is your standard pattern
    while(SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
        int col;
        printf("Found row\n");
        for(col=0; col<sqlite3_column_count(select_stmt); col++) {
            // Note that by using sqlite3_column_text, sqlite will coerce the value into a string
            printf("\tColumn %s(%i): '%s'\n",
                sqlite3_column_name(select_stmt, col), col,
                sqlite3_column_text(select_stmt, col));
        }
    }
    if(SQLITE_DONE != rc) {
        fprintf(stderr, "select statement didn't finish with DONE (%i): %s\n", rc, sqlite3_errmsg(db));
    } else {
        printf("\nSELECT successfully completed\n");
    }

    sqlite3_finalize(insert_stmt);
    sqlite3_finalize(select_stmt);
    sqlite3_close(db);
    return 0;
}
#endif
