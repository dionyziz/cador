#include "room.h"

map< unsigned int, Room* > Vilundo_Rooms;

///<static>///

Room* Room::Create( unsigned int roomid, string roomname ) {
    Room* rm;
    
    assert( Vilundo::Alphanumeric( roomname ) );
    assert( roomname.length() > 0 );
    
    Trace << "Setting up room `" << roomname << "' (" << roomid << ")";

    if ( !Vilundo::Alphanumeric( roomname ) ) {
        Warning << "Vilundo roomname is not alphanumeric (roomid: " << roomid;
        return 0;
    }
    if ( Vilundo_Rooms.find( roomid ) != Vilundo_Rooms.end() ) {
        Warning << "Duplicate vilundo roomid: " << roomid;
        return 0;
    }
    
    return Vilundo_Rooms[ roomid ] = new Room( roomid, roomname );
}

///</static>///

void Room::Init() {
    vector< string > rooms = conf->GetMultiSetting( "rooms" );
    vector< string > roomdata;
    unsigned int roomid;
    string roomname;

    for ( vector< string >::iterator i = rooms.begin(); i != rooms.end(); ++i ) {
        roomdata = explode( ":", *i, 2 );
        if ( roomdata.size() != 2 ) {
            Warning << "Invalid room configuration specified (no : character found)";
        }
        roomid = atoi( trim( roomdata[ 0 ] ).c_str() );
        roomname = trim( roomdata[ 1 ] );
        Room::Create( roomid, roomname );
    }
}

void Room::UserJoins( unsigned int userid ) {
    string s = "";
    unsigned char croomid[ 2 ];
    unsigned char cuserid[ 4 ];
    
    assert( userid > 0 );
    
    pair< set< unsigned int >::iterator, bool > r = this->mUsers.insert( userid );

    Trace << "User " << userid << " joining room " << this->mRoomName;
    if ( !r.second ) {
        Warning << "User " << userid << " is already in room " << this->mRoomName;
        return;
    }
    
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_JOINED );
    Vilundo::Encode4B( userid, cuserid );
    Vilundo::Encode2B( this->mRoomId, croomid );
    s.append( ( char* )cuserid, 4 );
    s.append( ( char* )croomid, 2 );
    
    Vilundo::Broadcast( s, this->mUsers );
}

void Room::UserParts( unsigned int userid ) {
    string s = "";
    unsigned char croomid[ 2 ];
    unsigned char cuserid[ 4 ];
    
    assert( userid > 0 );
    
    Trace << "User " << userid << " parting room " << this->mRoomName;
    if ( this->mUsers.erase( userid ) != 1 ) {
        Warning << "User " << userid << " is not in room " << this->mRoomName;
    }
    
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_PARTED );
    Vilundo::Encode4B( userid, cuserid );
    Vilundo::Encode2B( this->mRoomId, croomid );
    s.append( ( char* )cuserid, 4 );
    s.append( ( char* )croomid, 2 );
    
    Vilundo::Broadcast( s, this->mUsers );
}

void Room::UserMessage( unsigned int userid, string message ) {
    string s1 = "";
    string s2 = "";
    string s;
    unsigned char cmessageid[ 2 ];
    unsigned char cuserid[ 4 ];
    unsigned char croomid[ 2 ];

    if ( message.length() == 0 ) {
        Notice << "User " << userid << " attempted to send an empty message to room " << this->mRoomId << "; skipping";
        return;
    }
    
    if( !Vilundo::Alphanumeric( message ) ) {
        Notice << "User " << userid << " attempted to send non-alphanumeric message to room " << this->mRoomId << "; skipping";
        return;
    }

    Vilundo::AppendHeader( s1, VILUNDO_OUTGOING_ROOM_MESSAGE );
    Vilundo::Encode4B( userid, cuserid );
    s1.append( ( char* )cuserid, 4 );
    Vilundo::Encode2B( this->mRoomId, croomid );
    s1.append( ( char* )croomid, 2 );
    s2.append( message );
    s2.append( 1, '\0' );

    for ( set< unsigned int >::iterator i = this->mUsers.begin(); i != this->mUsers.end(); ++i ) {
        if ( *i != userid ) {
            s = "";
            s.append( s1 );
            Vilundo::Encode2B( Vilundo_FromUserId[ *i ]->NextMessageId(), cmessageid );
            s.append( ( char* )cmessageid, 2 );
            s.append( s2 );
            Vilundo_FromUserId[ *i ]->SendRawData( s );
        }
    }
}

Room::Room( unsigned int roomid, string roomname ) {
    assert( roomid > 0 );
    assert( Vilundo::Alphanumeric( roomname ) );
    assert( roomname.length() > 0 );
    
    this->mRoomId = roomid;
    this->mRoomName = roomname;
}

bool Room::UserInRoom( unsigned int userid ) {
    assert( userid > 0 );
    
    return this->mUsers.find( userid ) != this->mUsers.end();
}

set< unsigned int > Room::Users() {
    return this->mUsers;
}

string Room::Name() {
    return this->mRoomName;
}

