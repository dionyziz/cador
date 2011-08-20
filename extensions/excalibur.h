#ifndef CADOR_EXCALIBUR
#define CADOR_EXCALIBUR
#include "../extension.h"
#include "../conf.h"
#include "../error.h"
#include <string>
#include <mysql.h>
#include <sstream>

using namespace std;

class Cador_Excalibur : public Cador_Extension {
// class Cador_Excalibur {
    public:
        Cador_Excalibur();
        ~Cador_Excalibur();

        bool                Auth( int id, string auth );
        string              Username( int id );
        Vilundo_Privileges  Privileges( int id );
        void                Connect();
    private:
        bool                Query( string sql );
        MYSQL mDb;
};

#endif

