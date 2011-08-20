#include "pack.h"

string pack2b( int unpacked ) {
    char lo;
    char hi;
    string ret = "";
    
    lo = unpacked & 255;
    unpacked = unpacked >> 8;
    hi = unpacked & 255;
    ret.append(1, hi);
    ret.append(1, lo);

    return ret;
}

inline int unpack2b( string &packed ) {
    return packed[ 0 ] * 256 + packed[ 1 ];
}

string &pack4b( int unpacked ) {
    string ret = "";
    
    ret.append( pack2b( unpacked ) );
    unpacked = unpacked << 16;
    ret.append( pack2b( unpacked ) );
    
    return ret;
}

string unpack4b( string &packed ) {
    int ret;
    
    ret = unpack2b( packed.substr( 2 , 2 ) );
        + unpack2b( packed.substr( 0 , 2 ) ) * 65536;

    return ret;
}

