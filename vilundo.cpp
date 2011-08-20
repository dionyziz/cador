/*
    File: vilundo.cpp
    Description: Server-side implementation of the Vilundo protocol;
                 Vilundo class represent a single connection with a vilundo client.
    Developer: Dionyziz
*/

#include "vilundo.h"

map< unsigned int, Vilundo* > Vilundo_FromCommunicator;
map< unsigned int, Vilundo* > Vilundo_FromUserId;
CADOR_AUTH_EXTENSION* Vilundo_Auth;

///<static>///
void Vilundo::Broadcast( string message, set< unsigned int > userids ) {
    map< unsigned int, Vilundo* >::iterator j;

    assert( message.length() != 0 );
    if ( userids.size() == 0 ) {
        Trace << "Broadcast to noone";
        return;
    }
    
    for ( set< unsigned int >::iterator i = userids.begin(); i != userids.end(); ++i ) {
        j = Vilundo_FromUserId.find( *i );
        if ( j == Vilundo_FromUserId.end() ) {
            Warning << "Cannot broadcast to userid " << *i << "; user is not online";
            throw( 5 );
            continue;
        }
        j->second->SendRawData( message );
    }
}

inline string Vilundo::DisconnectReason2String( Vilundo_DisconnectError reason ) {
    switch ( reason ) {
        case VILUNDO_DISCONNECTERROR_USERQUITS:
            return "User is quitting";
        case VILUNDO_DISCONNECTERROR_CLIENTERROR:
            return "A client error occured";
        case VILUNDO_DISCONNECTERROR_KILLED:
            return "User was killed by a moderator";
        case VILUNDO_DISCONNECTERROR_BANNED:
            return "User is banned";
        case VILUNDO_DISCONNECTERROR_OVERLOADED:
            return "Server is overloaded";
        case VILUNDO_DISCONNECTERROR_UPGRADING:
            return "Server is upgrading";
        case VILUNDO_DISCONNECTERROR_SERVER:
            return "A server error occured";
        default:
            Notice << "Unknown disconnect reason";
            return "Unknown disconnect reason";
    }
}

void Vilundo::Init() {
    Vilundo_Auth = new CADOR_AUTH_EXTENSION();
}

string Vilundo::IncomingPacketHeader2String( Vilundo_IncomingPacketHeader fromhere ) {
    string s = "#";
    s += Vilundo::Packet2B2String( ( int )fromhere ) + "#";
    return s;
}

string Vilundo::OutgoingPacketHeader2String( Vilundo_OutgoingPacketHeader fromhere ) {
    string s = "#";
    s += Vilundo::Packet2B2String( ( int )fromhere ) + "#";
    return s;
}

string Vilundo::Packet2B2String( unsigned int fromhere ) {
    assert( fromhere >= 0 );
    assert( fromhere <= 65535 );
    
    return Vilundo::PacketB2String( fromhere >> 8 )
         + Vilundo::PacketB2String( fromhere & 255 );
}

string Vilundo::PacketB2String( unsigned int fromhere ) {
    string s = "";
    char tohere[ 2 ];
 
    assert( fromhere >= 0  );
    assert( fromhere <= 255 );
    
    tohere[ 0 ] = HEX[ fromhere >> 4 ];
    tohere[ 1 ] = HEX[ fromhere & 15 ];
    
    s.append( tohere, 2 );
    
    return s;
}

///</static>///

Vilundo::Vilundo() {
    Warning << "Constructing Vilundo using empty constructor!";
}

Vilundo::Vilundo( Communicator* cm ) {
    this->mCommunicator = cm;
    this->mAllowIncomingData = true;
    this->mClientIdentifier = "";
    this->mServerToClientMessageId = this->mClientToServerMessageId = 0;
    this->Initiate();
    Vilundo_FromCommunicator[ cm->SocketId() ] = this;
}

Vilundo::~Vilundo() {
}

string Vilundo::Id() {
    return this->mCommunicator->Id();
}

unsigned int Vilundo::NextMessageId() {
    if ( ++this->mServerToClientMessageId >= 65536 ) {
        this->mServerToClientMessageId = 1;
    }
    return this->mServerToClientMessageId;
}

void Vilundo::InitiationCompleted( const bool success, const string data ) {
    string s;

    if ( !success ) {
        Notice << this->Id() << ": Initiation timed out";
        this->SoftDisconnect();
        return;
    }
    if ( data != VILUNDO_CLIENT_INITIATE ) {
        Notice << this->Id() << ": Initiation Packet failed because of wrong data";
        this->SoftDisconnect();
        return;
    }
    s = "";
    s.append( VILUNDO_SERVER_INITIATE, VILUNDO_SERVER_INITIATE_ );
    this->mCommunicator->Send( s );
    Trace << this->Id() << ": Initiation completed";
    this->VersionHandshake();
}

void Vilundo::VersionHandshakeCompleted( const bool success, const string data ) {
    string s;

    if ( !success ) {
   	Notice << this->Id() << ": Version handshaking timed out";
        this->SoftDisconnect();
 	return;
    }
    s = "";
    s.append( VILUNDO_VERSION, 2 );
    if ( data != s ) {
        Notice << this->Id() << ": Version handshaking failed: Incompatible protocol version";
        this->SoftDisconnect();
        return;
    }
    Trace << this->Id() << ": Version handshaking succeeded";
    this->IdentifyClient();
}

void Vilundo::IdentifyClientCompleted( const bool success, const string data ) {
    if ( !success ) {
        Notice << this->Id() << ": Client identification failed";
        this->SoftDisconnect();
        return;
    }
    if ( !Vilundo::Alphanumeric( data ) ) {
        Notice << this->Id() << ": Client identification failed; client identifier is not alphanumeric";
        this->SoftDisconnect();
        return;
    }
    Trace << this->Id() << ": Client is using " << data;
    this->mClientIdentified = true;
    this->mClientIdentifier = "";
    this->mClientIdentifier.append( data );
    this->IdentifyServer();
}

void Vilundo::AuthenticationCompleted( const bool success, const string data ) {
    unsigned char cuserid[ 4 ];
    unsigned char cauthtoken[ 16 ];
    string s = "";

    Trace << this->Id() << ": Received authentication packet";

    assert( data.length() == 20 );

    for ( int i = 0; i < 4; ++i ) {
        cuserid[ i ] = data[ i ];
    }
    for ( int i = 0; i < 16; ++i ) {
        cauthtoken[ i ] = data[ i + 4 ];
    }

    this->mUserId = Vilundo::Decode4B( cuserid );
    Trace << this->Id() << ": Decoding authtoken";
    this->mAuthToken = Vilundo::DecodeAuthToken( cauthtoken );
    assert( this->mAuthToken.length() == 32 );
    Trace << this->Id() << ": Authtoken decoded (" << this->mAuthToken << ")";
    if ( this->Login() ) {
        Trace << this->Id() << ": Logged in as " << this->mUserId;
        Vilundo_FromUserId[ this->mUserId ] = this;
        this->MOTD();
        this->Awaken();
        return;
    }
    Notice << this->Id() << ": Login failure as " << this->mUserId;
    s.append( VILUNDO_AUTH_FAILURE, VILUNDO_AUTH_FAILURE_ );
    s.append( 1, ( char )VILUNDO_AUTHERROR_LOGIN_INCORRECT );
    this->mCommunicator->Send( s );
    this->SoftDisconnect();
}

void Vilundo::Awaken() {
    Trace << this->Id() << ": Waiting for a vilundo packet";
    this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::IncomingPacketHeaderDone );
}

void Vilundo::IncomingPacketHeaderCompleted( const bool success, const string data ) {
    unsigned char c[ 2 ];
    Vilundo_IncomingPacketHeader Packet;
    
    assert( data.length() == 2 );
    
    c[ 0 ] = data[ 0 ];
    c[ 1 ] = data[ 1 ];
    Packet = ( Vilundo_IncomingPacketHeader )Vilundo::Decode2B( c );
    
    string s = "";

    Trace << this->Id() << ": Received " << Vilundo::IncomingPacketHeader2String( Packet );

    switch ( Packet ) {
        case VILUNDO_INCOMING_REQUEST_MOTD:
            Trace << this->Id() << ": Received MOTD request";
            this->MOTD();
            break;
        case VILUNDO_INCOMING_REQUEST_JOIN:
            Trace << this->Id() << ": Received Join request";
            this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestJoinDone ); // roomid
            return;
        case VILUNDO_INCOMING_REQUEST_PART:
            Trace << this->Id() << ": Received Part request";
            this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestPartDone ); // roomid
            return;
        case VILUNDO_INCOMING_REQUEST_QUIT:
            Trace << this->Id() << ": Received Quit request";
            this->mCommunicator->WaitFor( 1, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestDisconnectDone ); // reason
            return;
        case VILUNDO_INCOMING_REQUEST_ACK:
            Trace << this->Id() << ": Received ACK request";
            this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestAckDone ); // ack id
            return;
        case VILUNDO_INCOMING_REQUEST_USERINFO:
            Trace << this->Id() << ": Received Userinfo request";
            s.append( "\0\0\0\0", 4 );
            this->mCommunicator->WaitUntil( s, 68, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestUserInfoDone );
            return;
        case VILUNDO_INCOMING_REQUEST_ROOMINFO:
            Trace << this->Id() << ": Received Roominfo request";
            s.append( "\0\0", 2 );
            this->mCommunicator->WaitUntil( s, 20, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestRoomInfoDone );
            return;
        case VILUNDO_INCOMING_REQUEST_USERLIST:
            Trace << this->Id() << ": Received Userlist request";
            s.append( "\0\0", 2 );
            this->mCommunicator->WaitUntil( s, 20, VILUNDO_PACKET_TIMEOUT, &Vilundo::RequestUserListDone );
            return;
        case VILUNDO_INCOMING_ROOM_MESSAGE:
            Trace << this->Id() << ": Received Room Message";
            this->mCommunicator->WaitFor( 4, VILUNDO_PACKET_TIMEOUT, &Vilundo::IncomingRoomMessageHeaderDone );
            return;
        case VILUNDO_INCOMING_ROOM_MESSAGE_OK:
            Trace << this->Id() << ": Room Message Delivered";
            this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::IncomingRoomMessageAckDone );
            return;
        default:
            Notice << this->Id() << ": Received unrecognized vilundo packet " << Vilundo::IncomingPacketHeader2String( Packet ) << "; ignoring";
            break;
    }
    this->Awaken();
}

void Vilundo::IncomingRoomMessageAckCompleted( const bool success, const string data ) {
    // TODO
    this->Awaken();
}

void Vilundo::RequestUserInfoCompleted( const bool success, const string data ) {
    unsigned char cuserid[ 4 ];
    unsigned int userid;
    unsigned int i;

    Trace << this->Id() << ": Request UserInfo completed";

    
    if ( !success ) {
        Notice << this->Id() << ": Invalid UserInfo request by client";
        this->SoftDisconnect();
        return;
    }
    
    for ( i = 0; i < data.length(); i += 4 ) {
        cuserid[ 0 ] = data[ i     ];
        cuserid[ 1 ] = data[ i + 1 ];
        cuserid[ 2 ] = data[ i + 2 ];
        cuserid[ 3 ] = data[ i + 3 ];
        userid = Vilundo::Decode4B( cuserid );
        this->SendUserInfo( userid );
    }
    assert( i <= 16 );
    if ( i == 0 ) {
        Notice << this->Id() << ": Client requested userinfo for an empty list of users; ignoring";
    }
    this->Awaken();
}

void Vilundo::RequestRoomInfoCompleted( const bool success, const string data ) {
    unsigned char croomid[ 2 ];
    unsigned int roomid;
    unsigned int i;

    Trace << this->Id() << ": Request RoomInfo completed";

    if ( !success ) {
        Notice << this->Id() << ": Invalid RoomInfo request by client";
        this->SoftDisconnect();
        return;
    }

    for ( i = 0; i < data.length(); i += 4 ) {
        croomid[ 0 ] = data[ i     ];
        croomid[ 1 ] = data[ i + 1 ];
        roomid = Vilundo::Decode2B( croomid );
        this->SendRoomInfo( roomid );
    }
    assert( i <= 8 );
    if ( i == 0 ) {
        Notice << this->Id() << ": Client requested roominfo for an empty list of rooms; ignoring";
    }
    this->Awaken();
}

void Vilundo::RequestUserListCompleted( const bool success, const string data ) {
    unsigned char croomid[ 2 ];
    unsigned int roomid;
    unsigned int i;
    
    if ( !success ) {
        Notice << this->Id() << ": Invalid UserList request by client";
        this->SoftDisconnect();
        return;
    }
    
    for ( i = 0; i < data.length(); i += 2 ) {
        croomid[ 0 ] = data[ i     ];
        croomid[ 1 ] = data[ i + 1 ];
        roomid = Vilundo::Decode2B( croomid );
        this->SendUserList( roomid );
    }
    assert( i <= 8 );
    if ( i == 0 ) {
        Notice << this->Id() << ": Client requested roominfo for an empty list of rooms; ignoring";
    }
    this->Awaken();
}

void Vilundo::SendUserInfo( unsigned int userid ) {
    string s = "";
    string username = "";
    unsigned char cuserid[ 4 ];
    Vilundo_Privileges priv;

    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_USERINFO );
    Vilundo::Encode4B( userid, cuserid );
    s.append( ( char* )cuserid, 4 );
    username = Vilundo_Auth->Username( userid );
    if ( !username.length() ) { // user not found
        Notice << this->Id() << ": Userinfo request for non-existing user " << userid;
        s.append( 1, ( char )VILUNDO_PRIVILEGES_NOT_FOUND );
        this->mCommunicator->Send( s );
        return;
    }
    priv = ( Vilundo_Privileges )Vilundo_Auth->Privileges( userid );
    s.append( 1, ( char )priv );
    assert( Vilundo::Alphanumeric( username ) );
    s.append( username );
    s.append( 1, '\0' );
    this->mCommunicator->Send( s );
}

void Vilundo::SendRoomInfo( unsigned int roomid ) {
    string s = "";
    unsigned char croomid[ 2 ];
    string roomname;
    map< unsigned int, Room* >::iterator i = Vilundo_Rooms.find( roomid );

    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_ROOMINFO );
    Vilundo::Encode2B( roomid, croomid );
    s.append( ( char* )croomid, 2 );
    if ( i == Vilundo_Rooms.end() ) {
        s.append( 1, ( char )VILUNDO_PRIVILEGES_NOT_FOUND );
    }
    else {
        s.append( 1, ( char )VILUNDO_PRIVILEGES_USER );
        roomname = i->second->Name();
        s.append( roomname );
    }
    s.append( 1, '\0' );
    this->mCommunicator->Send( s );
}

void Vilundo::SendUserList( unsigned int roomid ) {
    string s = "";
    set< unsigned int > users;
    unsigned char croomid[ 2 ];
    unsigned char cuserid[ 4 ];
    
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_USERLIST );
    Vilundo::Encode2B( roomid, croomid );
    s.append( ( char* )croomid, 2 );
    s.append( 1, '\0' );
    users = Vilundo_Rooms[ roomid ]->Users();
    Trace << "(listing users)";
    for ( set< unsigned int >::iterator i = users.begin(); i != users.end(); ++i ) {
        Trace << "(user list member: " << *i << ")";
        Vilundo::Encode4B( *i, cuserid );
        s.append( ( char* )cuserid, 4 );
    }
    Trace << "(end of user list)";
    s.append( "\0\0\0\0", 4 );
    this->mCommunicator->Send( s );
}

void Vilundo::IncomingRoomMessageHeaderCompleted( const bool success, string data ) {
    string s = "";
    unsigned int roomid;
    unsigned int messageid;
    unsigned char croomid[ 2 ];
    unsigned char cmessageid[ 2 ];
    
    croomid[ 0 ] = data[ 0 ];
    croomid[ 1 ] = data[ 1 ];
    roomid = Vilundo::Decode2B( croomid );
    cmessageid[ 0 ] = data[ 2 ];
    cmessageid[ 1 ] = data[ 3 ];
    messageid = Vilundo::Decode2B( cmessageid );
    if ( roomid <= 0 || roomid >= 65536 ) {
        Notice << this->Id() << ": Received invalid roomid: " << Vilundo::Packet2B2String( roomid );
        this->SoftDisconnect();
        return;
    }
    Trace << this->Id() << ": (messageid: " << Vilundo::Packet2B2String( messageid ) << ")";
    if ( messageid <= 0 || messageid >= 65536 ) {
        Notice << this->Id() << ": Received invalid messageid: " << Vilundo::Packet2B2String( messageid );
        this->SoftDisconnect();
        return;
    }
    this->mPhaseId1 = roomid;
    this->mPhaseId2 = messageid;
    s.append( 1, '\0' );
    this->mCommunicator->WaitUntil( s, 2048, VILUNDO_PACKET_TIMEOUT, &Vilundo::IncomingRoomMessageDone );
}

void Vilundo::IncomingRoomMessageCompleted( const bool success, string data ) {
    string message;
    string s = "";
    unsigned int roomid;
    unsigned int messageid;
    unsigned char cmessageid[ 2 ];
    map< unsigned int, Room* >::iterator i;
    
    roomid = this->mPhaseId1;
    messageid = this->mPhaseId2;

    i = Vilundo_Rooms.find( roomid );
    
    if ( i == Vilundo_Rooms.end() ) {
        Notice << this->Id() << ": Client attempted to message to a non-existing room " << roomid;
        this->Awaken();
        return;
    }
    
    message = data;
    roomid = this->mPhaseId1;
    messageid = this->mPhaseId2;
    assert( roomid > 0 && roomid < 65536 );
    assert( messageid > 0 && messageid < 65536 );
    Vilundo_Rooms[ roomid ]->UserMessage( this->mUserId, message );
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_ROOM_MESSAGE_OK );
    Vilundo::Encode2B( messageid, cmessageid );
    s.append( ( char* )cmessageid, 2 );
    this->mCommunicator->Send( s );
    this->Awaken();
}

void Vilundo::RequestJoinCompleted( const bool success, string data ) {
    assert( data.length() == 2 );
    
    unsigned int roomid = Vilundo::Decode2B( ( unsigned char* )data.c_str() );
    map< unsigned int, Room* >::iterator i = Vilundo_Rooms.find( roomid );
    
    if ( i == Vilundo_Rooms.end() ) {
        Notice << this->Id() << ": Client attempted to join non-existing room (roomid: " << roomid << ")";
        this->JoinFailure( roomid, VILUNDO_JOINROOMERROR_NO_SUCH_ROOM );
        return;
    }
    if ( i->second->UserInRoom( this->mUserId ) ) {
        Notice << this->Id() << ": Client attempted to re-join room while already in room (roomid: " << roomid << ")";
        this->JoinFailure( roomid, VILUNDO_JOINROOMERROR_ALREADY_JOINED );
        return;
    }
    this->JoinSuccess( i->second );
}

void Vilundo::JoinFailure( unsigned int roomid, Vilundo_JoinRoomError reason ) {
    string s = "";
    
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_JOIN_FAILURE );
    s.append( 1, ( char )reason );
    this->mCommunicator->Send( s );
    this->Awaken();
}

void Vilundo::JoinSuccess( Room* room ) {
    room->UserJoins( this->mUserId );
    this->Awaken();
}

void Vilundo::RequestPartCompleted( const bool success, string data ) {
    assert( data.length() == 2 );
    
    unsigned int roomid = Vilundo::Decode2B( ( unsigned char* )data.c_str() );
    map< unsigned int, Room* >::iterator i = Vilundo_Rooms.find( roomid );
    
    if ( i == Vilundo_Rooms.end() ) {
        Notice << this->Id() << ": Client attempted to part non-existing room";
        this->PartFailure( roomid, VILUNDO_PARTROOMERROR_NO_SUCH_ROOM );
        return;
    }
    if ( !i->second->UserInRoom( this->mUserId ) ) {
        Notice << this->Id() << ": Client attempted to part room while not in room";
        this->PartFailure( roomid, VILUNDO_PARTROOMERROR_ALREADY_PARTED );
        return;
    }
    this->PartSuccess( i->second );
}

void Vilundo::PartFailure( unsigned int roomid, Vilundo_PartRoomError reason ) {
    string s = "";
    
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_PART_FAILURE );
    s.append( 1, ( char )reason );
    this->mCommunicator->Send( s );
    this->Awaken();
}

void Vilundo::PartSuccess( Room* room ) {
    room->UserParts( this->mUserId );
    this->Awaken();
}

void Vilundo::RequestDisconnectCompleted( const bool success, const string data ) {
    assert( data.length() == 1 );
    
    Vilundo_DisconnectError reason = ( Vilundo_DisconnectError )data[ 0 ];
    
    Trace << this->Id() << ": Client disconnecting (" << Vilundo::DisconnectReason2String( reason ) << ")";
    this->SoftDisconnect();
}

void Vilundo::Kill( Vilundo_DisconnectError reason ) {
    string s = "";
    
    Trace << this->Id() << ": Killing client (" << Vilundo::DisconnectReason2String( reason ) << ")";
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_REQUEST_QUIT );
    s.append( 1, ( char )reason );
    this->mCommunicator->Send( s );
    this->SoftDisconnect();
}

void Vilundo::RequestAckCompleted( const bool success, const string data ) {
    assert( data.length() == 2 );
    
    string s = "";

    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_ACK );
    s.append( data );
    this->mCommunicator->Send( s );
    this->Awaken();
}

void Vilundo::MOTD() {
    string s = "";
    string motd = "";
    
    Trace << this->Id() << ": Sending MOTD";
    motd = conf->GetSetting( "motd" );
    if ( !Vilundo::Alphanumeric( motd ) ) {
        Warning << "MOTD is not alphanumeric!";
        return;
    }
    if ( motd.length() > 1024 ) {
        Warning << "MOTD is too long (>1024B)";
        return;
    }
    Vilundo::AppendHeader( s, VILUNDO_OUTGOING_MOTD );
    s.append( motd );
    s.append( 1, '\x00' );
    this->mCommunicator->Send( s );
}

void Vilundo::SoftDisconnect() {
    Trace << this->Id() << ": Soft-terminating connection with client (timeout in " << VILUNDO_CONNECTION_TIMEOUT << " ms)";
    this->mAllowIncomingData = false;
    // TODO: connection timeout to hard-close the connection
}

void Vilundo::Initiate() {
    Trace << this->Id() << ": Initiating";
    this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::InitiationDone );
}

void Vilundo::VersionHandshake() {
    string s;

    Trace << this->Id() << ": Version handshaking";
    s.append( VILUNDO_VERSION, 2 );
    this->mCommunicator->Send( s );
    this->mCommunicator->WaitFor( 2, VILUNDO_PACKET_TIMEOUT, &Vilundo::VersionHandshakeDone );
}

void Vilundo::IdentifyClient() {
    string s;

    s.append( "\0", 1 );
    this->mCommunicator->WaitUntil( s, 255, VILUNDO_PACKET_TIMEOUT, &Vilundo::IdentifyClientDone );
}

void Vilundo::IdentifyServer() {
    string s;

    s.append( VILUNDO_SERVER_ID );
    s.append( 1, '\0' );
    this->mCommunicator->Send( s );
    this->mServerIdentified = true;
    this->Authenticate();
}

void Vilundo::Authenticate() {
    this->mCommunicator->WaitFor( 20, VILUNDO_PACKET_TIMEOUT, &Vilundo::AuthenticationDone );
}

bool Vilundo::Login() {
    Trace << this->Id() << ": Logging in as user " << this->mUserId;

    return Vilundo_Auth->Auth( this->mUserId, this->mAuthToken );
}

void Vilundo::SendRawData( string data ) {
    this->mCommunicator->Send( data );
}

void Vilundo::Run() {
    this->mCommunicator->Run();
}

