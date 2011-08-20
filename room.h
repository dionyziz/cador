#ifndef CADOR_ROOM
#define CADOR_ROOM
#include <set>
#include <map>
#include "cador.h"
#include "config.h"
#include "string.h"
#include "vilundo.h"

class Room {
    public:
        Room( unsigned int, string );
        ~Room();
        static Room* Create( unsigned int, string );
        static void Init();
        void UserJoins( unsigned int );
        void UserParts( unsigned int );
        void UserMessage( unsigned int, string );
        void RoomCreated();
        void RoomDestroyed();
        bool UserInRoom( unsigned int );
        set< unsigned int > Users();
        string Name();
    private:
        unsigned int mRoomId;
        string mRoomName;
        set< unsigned int > mUsers; // userids
};

extern map< unsigned int, Room* > Vilundo_Rooms; /* list of all rooms roomid -> Room */

#endif

