#ifndef CADOR_DUMMY
#define CADOR_DUMMY
#include "../extension.h"
// #include "../conf.h"
#include "../string.h"

class Cador_Dummy: public Cador_Extension {
    public:
        Cador_Dummy();
        ~Cador_Dummy();

        bool Auth( int, string );
        string Username( int );
        Vilundo_Privileges Privileges( int );
};

#endif

