#ifndef CADOR_VILUNDO
#define CADOR_VILUNDO

#include "network.h"
#include "user.h"
#include "room.h"
#include "headers.h"
#include "config.h"
#include <pcrecpp.h>
#include <map>

using namespace std;
using namespace pcrecpp;

const int VILUNDO_PACKET_TIMEOUT = 5000;
const int VILUNDO_CONNECTION_TIMEOUT = 60000;

const char* const VILUNDO_SERVER_ID = "Kamibu Cador"; // daemon identifier
const char* const HEX = "0123456789abcdef";

class Vilundo;
class Room;

extern map< unsigned int, Vilundo* > Vilundo_FromCommunicator;

class Vilundo {
    public:
        Vilundo();
        Vilundo( Communicator* );
        ~Vilundo();
        void Construct();
        void SoftDisconnect();
        void Initiate();
        void Run();
        void VersionHandshake();
        void IdentifyClient();
        void IdentifyServer();
        void Authenticate();
        string Id();
        void SendRawData( string );
        unsigned int NextMessageId();

        // <static>
        static inline bool Alphanumeric( const string s ) {
            return RE( "^[a-zA-Z0-9\\ \\.\\\\\\+\\*\\?\\[\\^\\]\\$\\(\\)\\{\\}\\=\\!\\<\\>\\|\\:\\[\\]\\-,@#%&`~_/\"';]*$" ).FullMatch( s.c_str() ); 
        }

        static inline unsigned int Decode2B( const unsigned char* const fromhere ) {
            return ( ( int )fromhere[ 0 ] << 8 )
                   + ( int )fromhere[ 1 ];
        }

        static inline void Encode2B( const unsigned int id, unsigned char* const tohere ) {
            assert( id >= 0 );
            assert( id <= 65535 );
            
            tohere[ 1 ] =   id & 255;
            tohere[ 0 ] = ( id >> 8 ) & 255;
        }

        static inline unsigned int Decode4B( const unsigned char* const fromhere ) {
            return ( ( int )fromhere[ 0 ] << 24 )
                 + ( ( int )fromhere[ 1 ] << 16 )
                 + ( ( int )fromhere[ 2 ] << 8  )
                 +   ( int )fromhere[ 3 ];
        }

        static inline void Encode4B( const unsigned int id, unsigned char* tohere ) {
            assert( id >= 0 );
            
            tohere[ 3 ] =   id & 255;
            tohere[ 2 ] = ( id >> 8  ) & 255;
            tohere[ 1 ] = ( id >> 16 ) & 255;
            tohere[ 0 ] = ( id >> 24 ) & 255;
        }

        static inline string DecodeAuthToken( const unsigned char* const fromhere ) {
            unsigned int i;
            unsigned int hi;
            unsigned int lo;
            unsigned char c;
            char tohere[ 32 ];
            string s = "";

            for ( i = 0; i < 16; ++i ) {
                c = fromhere[ i ];
                hi = c >> 4;
                lo = c & 15;
                if ( hi < 0 ) {
                    Warning << "hi is < 0: " << hi << " / " << ( int )c;
                }
                assert( hi >= 0 );
                assert( hi < 16 );
                assert( lo >= 0 );
                assert( lo < 16 );
                tohere[ 2 * i     ] = HEX[ hi ];
                tohere[ 2 * i + 1 ] = HEX[ lo ];
            }
            s.append( tohere, 32 );

            return s;
        }

	    static string PacketB2String( const unsigned int );
        static string Packet2B2String( const unsigned int );
        static string IncomingPacketHeader2String( Vilundo_IncomingPacketHeader );
        static string OutgoingPacketHeader2String( Vilundo_OutgoingPacketHeader );
        static void Broadcast( string, set< unsigned int > );
        static inline string DisconnectReason2String( Vilundo_DisconnectError );
        static void Init();

        static inline void AppendHeader( string &s, Vilundo_OutgoingPacketHeader h ) {
            unsigned char c[ 2 ];
            
            Trace << "(preparing to send " << Vilundo::OutgoingPacketHeader2String( h ) << ")";
            Vilundo::Encode2B( h, c );
            s.append( ( char* )c, 2 );
        }

        // callback wrappers
        static inline void InitiationDone               ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->InitiationCompleted               ( success, data );        }
        static inline void VersionHandshakeDone         ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->VersionHandshakeCompleted         ( success, data );        }
        static inline void IdentifyClientDone           ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->IdentifyClientCompleted           ( success, data );        }
        static inline void AuthenticationDone           ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->AuthenticationCompleted           ( success, data );        }
        static inline void IncomingPacketHeaderDone     ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->IncomingPacketHeaderCompleted     ( success, data );        }
        static inline void RequestJoinDone              ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestJoinCompleted              ( success, data );        }
        static inline void RequestPartDone              ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestPartCompleted              ( success, data );        }
        static inline void RequestDisconnectDone        ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestDisconnectCompleted        ( success, data );        }
        static inline void RequestAckDone               ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestAckCompleted               ( success, data );        }
        static inline void RequestUserInfoDone          ( Communicator* C, const bool success, const string data ) 
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestUserInfoCompleted          ( success, data );        }
        static inline void RequestRoomInfoDone          ( Communicator* C, const bool success, const string data )
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestRoomInfoCompleted          ( success, data );        }
        static inline void RequestUserListDone          ( Communicator* C, const bool success, const string data )
        { Vilundo_FromCommunicator[ C->SocketId() ]->RequestUserListCompleted          ( success, data );        }
        static inline void IncomingRoomMessageHeaderDone( Communicator* C, const bool success, const string data )
        { Vilundo_FromCommunicator[ C->SocketId() ]->IncomingRoomMessageHeaderCompleted( success, data );        }
        static inline void IncomingRoomMessageDone      ( Communicator* C, const bool success, const string data )
        { Vilundo_FromCommunicator[ C->SocketId() ]->IncomingRoomMessageCompleted      ( success, data );        }
        static inline void IncomingRoomMessageAckDone   ( Communicator* C, const bool success, const string data )
        { Vilundo_FromCommunicator[ C->SocketId() ]->IncomingRoomMessageAckCompleted   ( success, data );        }
        // </static>
    protected:
        // actual callbacks
        void InitiationCompleted               ( const bool, const string );
        void VersionHandshakeCompleted         ( const bool, const string );
        void IdentifyClientCompleted           ( const bool, const string );
        void AuthenticationCompleted           ( const bool, const string );
        void IncomingPacketHeaderCompleted     ( const bool, const string );
        void RequestJoinCompleted              ( const bool, const string );
        void RequestPartCompleted              ( const bool, const string );
        void RequestDisconnectCompleted        ( const bool, const string );
        void RequestAckCompleted               ( const bool, const string );
        void RequestUserInfoCompleted          ( const bool, const string );
        void RequestRoomInfoCompleted          ( const bool, const string );
        void RequestUserListCompleted          ( const bool, const string );
        void IncomingRoomMessageHeaderCompleted( const bool, const string );
        void IncomingRoomMessageCompleted      ( const bool, const string );
        void IncomingRoomMessageAckCompleted   ( const bool, const string );
        bool Login(); // checks mUserId/mAuthToken
        void SendUserInfo( unsigned int );
        void SendRoomInfo( unsigned int );
        void SendUserList( unsigned int );
        void MOTD();
        void Awaken();
        void JoinFailure( unsigned int, Vilundo_JoinRoomError );
        void JoinSuccess( Room* );
        void PartFailure( unsigned int, Vilundo_PartRoomError );
        void PartSuccess( Room* );
        void Kill( Vilundo_DisconnectError );
    private:
        unsigned int mUserId;
        unsigned short int mVersion;
        Communicator* mCommunicator;
        bool mAuthenticated;
        bool mInitiated;
        bool mVersionHandshaked;
        bool mClientIdentified;
        bool mServerIdentified;
        bool mAllowIncomingData;
        unsigned int mPhaseId1; // temporary holders
        unsigned int mPhaseId2;
        unsigned int mServerToClientMessageId; // latest
        unsigned int mClientToServerMessageId; // latest
        string mClientIdentifier;
        string mAuthToken;
};

extern map< unsigned int, Vilundo* > Vilundo_FromUserId;

#endif

