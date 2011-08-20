/* vim: set expandtab tabstop=4 shiftwidth=4 softtabstop=4: */
#ifndef CADOR_EXTENSION
#define CADOR_EXTENSION
#include "string.h"
#include "headers.h"

class Cador_Extension {
    public:
        virtual bool Auth( int, string ) = 0;
        virtual string Username( int ) = 0;
        virtual Vilundo_Privileges Privileges( int ) = 0;
};

#endif
