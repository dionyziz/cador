#include <iostream>

using namespace std;

const char* const PROTOCOL = "VL";
const char* const VERSION = "\x01\x01";
const char* const CLIENT = "Kamibu CadorTest";
const char* const AUTHTOKEN = "e151ff006f3bf3449c55130655147bd6";
const char* const HEX = "0123456789abcdef";

int hex2dec( const char c ) {
    for ( int i = 0; i < 16; ++i ) {
        if ( HEX[ i ] == c ) {
            return i;
        }
    }
    return -1;
}

void Vilundo_AuthToken_Decode( const char* const token, char* tohere ) {
    for ( int i = 0; i < 16; ++i ) {
        tohere[ i ] = hex2dec( token[ i * 2 ] ) << 4 | hex2dec( token[ i * 2 + 1 ] );
    }
}

int main() {
    char* rawtoken = ( char* )malloc( 16 );
    
    cout << PROTOCOL;
    cout << VERSION;
    cout << CLIENT << '\x00';
    cout << '\x00' << '\x00' << '\x00' << '\x01'; // authentication userid
    Vilundo_AuthToken_Decode( AUTHTOKEN, rawtoken );
    for ( int i = 0; i < 16; ++i ) {
        cout << rawtoken[ i ];
    }
    cout << '\x00' << '\x03'; // join
    cout << '\x00' << '\x45'; // room #69

    cout << '\x10' << '\x00' << '\x00' << '\x45' << '\x00' << '\x00'; // userlist

    cout << '\x00' << '\x0c'; // request userinfo
    cout << '\x00' << '\x00' << '\x00' << '\x01'; // for user #1
    cout << '\x00' << '\x00' << '\x00' << '\x00'; // eof
    cout << '\x00' << '\x0c'; // request userinfo
    cout << '\x00' << '\x01' << '\x01' << '\xfd'; // non-existing user
    cout << '\x00' << '\x00' << '\x00' << '\x00'; // eof

    cout << '\x00' << '\x18'; // send room message
    cout << '\x00' << '\x45'; // to room #69
    cout << '\x00' << '\x01'; // message id #1
    cout << "Hello, world!";
    cout << '\x00'; // eof

    return 0;
}

