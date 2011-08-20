#include <mysql.h>
#include <iostream>

using namespace std;

int main() {
    MYSQL* pConnection;

    if ( NULL == ( pConnection = mysql_init( NULL ) ) ) {
        cout << "Failed to mysql_init()" << endl;
        return 1;
    }

    if ( NULL == mysql_real_connect( pConnection, "localhost", "root", "", "test", 0, NULL, 0 ) ) {
        cout << "Failed to mysql_real_connect(); check your password?" << endl;
    }

    return 0;
}

