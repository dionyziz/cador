#include <cstdio>
#include <iostream>

using namespace std;

const char* const HEX = "0123456789abcdef";

static inline string PacketHeader2String( int fromhere ) {
    int i;
    char c;
    char tohere[ 4 ];
    int hi;
    int lo;
    string s;

    s = "#";
    hi = fromhere >> 8;
    lo = fromhere & 255;
    tohere[ 0 ] = HEX[ hi >> 4 ];
    tohere[ 1 ] = HEX[ hi & 15 ];
    tohere[ 2 ] = HEX[ lo >> 4 ];
    tohere[ 3 ] = HEX[ lo & 15 ];
    s.append( tohere, 4 );
    s += "#";

    return s;
}

int main() {
    cout << PacketHeader2String( 0x0080 ) << endl;

    return 0;
}

