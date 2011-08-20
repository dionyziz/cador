#ifndef CADOR_CONFIG
#define CADOR_CONFIG

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility> 
#include "string.h"
#include "xerces.h"

using namespace std;

class Config {
    public:
        Config();
        ~Config();
        string GetSetting( string );
        string GetSetting( char* );
        vector< string > GetMultiSetting( char* );
        void SetDefaultSetting( string , string );
        void SetDefaultSetting( char* , char* );
        void SetSetting( string , string );
        void SetSetting( char* , char* );
        void LoadFromFile( const string );
    private:
        void Parse();
        void ParseLine( string );
        void MultiSettingBegin( string );
        void MultiSettingAdd( string );
        void MultiSettingEnd( string );
        string mFilename;
        map< string, string > mConf;
        map< string, vector< string > > mMultiConf;
        map< string, vector< string > >::iterator mCurrentMultiSetting;
};

extern Config* conf;

#endif
