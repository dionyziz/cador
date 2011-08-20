#include "excalibur.h"

Cador_Excalibur::Cador_Excalibur() : Cador_Extension() {
    // Cador_Excalibur::Cador_Excalibur() {
    this->Connect();
}

void Cador_Excalibur::Connect() {
    mysql_init( &mDb ); // initialize connection to database

    string dbname = "dbname";
    // try connecting to database
    if ( !mysql_real_connect( &mDb,         // the MYSQL structure
                              "localhost",  // host
                              "username",       // username
                              "password",           // password
                              dbname.c_str(),   // database
                              0,            // port
                              NULL,         // unix_socket
                              0 ) ) {       // flag

        Warning << "Failed to connect to database: Error: " << mysql_error( &mDb );
    }
    
    Trace << "Excalibur: Successfully connected to database " << dbname;
}

Cador_Excalibur::~Cador_Excalibur() {}

bool Cador_Excalibur::Auth( int id, string authtoken ) {
    ostringstream sql;                              // our sql query

    Trace << "Excalibur: authenticating user " << id << " with authtoken " << authtoken;

    sql << "SELECT                                          \
                *                                           \
            FROM                                            \
                `merlin_users`                              \
            WHERE                                           \
                `user_id` = '" << id << "' AND              \
                `user_authtoken` = '" << authtoken << "'    \
            LIMIT 1";

    if ( !this->Query( sql.str() ) ) {              // try running the query
        return false;                               // return false if it failed
    }

    MYSQL_RES * res = mysql_store_result( &mDb );   // get mysql resource

    if ( mysql_num_rows( res ) ) {
        Trace << "Excalibur: authentication successful";

        return true;
    }

    Trace << "Excalibur: authentication failed: wrong pair";
    return false;
}

string Cador_Excalibur::Username( int id ) {
    ostringstream sql;

    Trace << "Excalibur: fetching username of user " << id;
    sql << "SELECT                          \
                `user_name`                 \
            FROM                            \
                `merlin_users`              \
            WHERE                           \
                `user_id` = '" << id << "'  \
            LIMIT 1";

    if ( !this->Query( sql.str() ) ) {              // try running the query
        return false;                               // if it failed, return false
    }

    MYSQL_RES * res = mysql_store_result( &mDb );   // get the stored resource
    MYSQL_ROW row   = mysql_fetch_row( res );       // fetch the row

    Trace << "Excalibur: username fetched successfully";

    return row[ 0 ];                                // return the username (first column of the row)
}

Vilundo_Privileges Cador_Excalibur::Privileges( int id ) {
    ostringstream sql;

    Trace << "Excalibur: Fetching permissions of user " << id;

    sql << "SELECT \
                `user_rights` \
            FROM \
                `merlin_users` \
            WHERE \
                `user_id` = '" << id << "' \
            LIMIT 1;";

    if ( !this->Query( sql.str() ) ) {                   // try running the query
        return VILUNDO_PRIVILEGES_NOT_FOUND;             // if it failed, return respective constant
    }

    MYSQL_RES * res = mysql_store_result( &mDb );        // get the stored resource
    MYSQL_ROW row   = mysql_fetch_row( res );            // fetch the row

    Trace << "Excalibur: Fetching permissions successful";

    string rights = row[ 0 ];                            // rights is the first column of the row

    return ( Vilundo_Privileges )atoi( rights.c_str() ); // convert rights to integer and return
}

bool Cador_Excalibur::Query( string sql ) {
    // try running the query
    if ( mysql_query( &mDb, sql.c_str() ) ) {
        if ( mysql_errno( &mDb ) == 2006 ) { // mysql timeout, let's reconnect
            this->Connect();            
            return this->Query( sql );
        }
        Warning << "MySQL query failed: Error: " << mysql_error( &mDb );
        return false;   // failure :/
    }

    return true;        // success!
}

