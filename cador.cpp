/*
    File: cador.cpp
    Description: A Vilundo daemon
    Developer: Dionyziz
*/
// #define NDEBUG
#include "cador.h"

vector< Vilundo* > clients;
Server* VL;
set< string > IPBans;

void Cador_Run();
void Cador_Construct();
void Cador_Destruct();

int main() {
    Trace << "Starting Cador Vilundo Server";

    Cador_Construct();
    Cador_Run();
    Cador_Destruct();
    
    return 0;
}

void Cador_Run() {
    while ( true ) {
        sleep( 1 );
        VL->Run();
        for ( vector< Vilundo* >::iterator c = clients.begin(); c != clients.end(); ++c ) {
            ( *c )->Run();
        }
    }
}

void Cador_LoadConfig() {
    conf = new Config();

    conf->SetDefaultSetting( "port", "24202" );
    conf->SetDefaultSetting( "backlog", "10" );
    conf->SetDefaultSetting( "motd", "" );
    conf->LoadFromFile( "cador.conf" );
    
    Trace << "Config Loaded";
}

void Cador_ConnectionReceived( Server* S, Communicator* C ) {
    Vilundo* V;
    
    V = new Vilundo( C );
    clients.push_back( V );
}

void Cador_Construct() {
    Cador_LoadConfig();
    Vilundo::Init();
    VL = new Server();
    VL->SetPort( atoi( conf->GetSetting( "port" ).c_str() ) );
    VL->Listen();
    VL->SetAcceptCallback( Cador_ConnectionReceived );
    Room::Init();
}

void Cador_Destruct() {
    delete VL;
}

