#include <iostream>
#include "vilundo.h"
#include "error.h"

using namespace std;

template < typename T >
void test( string name, T expected, T actual ) {
    if ( expected != actual ) {
        Warning << "FAIL: " << name << "\n" << "Expected: " << expected << "\nActual: " << actual << "\n";
        return;
    }
    Trace << "PASS: " << name;
}

int main() {
    string s;
    string t;
    char c[ 4 ];

    s = "hello";
    t = "hello";
    test( "assert", 0, 0 );
    test( "assert", true, true );
    test( "assert", s, t );
    test( "assert", s.c_str()[ 0 ], t.c_str()[ 0 ] );
    Vilundo_Encode4B( 5, c );
    test( "userid encode/decode", 5, Vilundo_Decode4B( c ) );
    test( "alphanumerosity", true, Vilundo_Alphanumeric( "Hello, world!" ) );
    test( "nonalphanumerosity", false, Vilundo_Alphanumeric( "\x01" ) );

    cout << endl;

    return 0;
}

