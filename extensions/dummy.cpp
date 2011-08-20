#include "dummy.h"

Cador_Dummy::Cador_Dummy() {
}

Cador_Dummy::~Cador_Dummy() {
}

bool Cador_Dummy::Auth( int id, string pass ) {
    if ( id < 2048 ) {
        return true;
    }
    return false;
}

string Cador_Dummy::Username( int id ) {
    char c[ 10 ];
    string s;

    if ( id >= 2048 ) {
        return "";
    }

    sprintf( c, "%i", id );
    s = c;

    return s;
}

Vilundo_Privileges Cador_Dummy::Privileges( int id ) {
    if ( id < 2048 ) {
        return VILUNDO_PRIVILEGES_USER;
    }
    return VILUNDO_PRIVILEGES_NOT_FOUND;
}

