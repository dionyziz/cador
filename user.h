#ifndef CADOR_USER
#define CADOR_USER
#include "string.h"

const char CADOR_HEX[] = "0123456789abcdef";

inline string Vilundo_AuthToken_Decode( char rawauthtoken[ 16 ] /* raw binary */ ) {
    string ret;
    char c;

    for ( int i = 0; i < 16; ++i ) {
        c = rawauthtoken[ i ];
        ret += CADOR_HEX[ c & ~4 ];
        ret += CADOR_HEX[ c &  4 ];
    }

    return ret;
}

class User {
    public:
        int UserId() {
            return this->mUserId;
        }
        string Username() {
            return this->mUsername;
        }
        string AuthToken() {
            return this->mAuthToken;
        }
    private:
        int mUserId;
        string mUsername;
        string mAuthToken;
};

#endif

