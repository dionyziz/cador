/*
    File: config.cpp
    Description: A simple config file parser
    Developer: Dionyziz
*/
#include "config.h"
#include "error.h"

Config* conf;

Config::Config() {
    this->mFilename = "";
}

void Config::LoadFromFile( const string filename ) {
    this->mFilename = filename;
    this->mCurrentMultiSetting = this->mMultiConf.end();
    this->Parse();
}

Config::~Config() {
}

void Config::Parse() {
    ifstream fi;
    char c_str_line[ 65535 ];
    string line;
    
    fi.open( this->mFilename.c_str() );
    if ( !fi.is_open() ) {
        Notice << "Configuration file " << this->mFilename << " could not be opened; using default settings";
        return;
    }
    while ( !fi.eof() ) {
        fi.getline( c_str_line, 65535 );
        line = c_str_line; // convert to string
        this->ParseLine( line );
    }
    fi.close();
    if ( this->mCurrentMultiSetting != this->mMultiConf.end() ) {
        Warning << "Configuration file " << this->mFilename << " contains unclosed section " << this->mCurrentMultiSetting->first;
    }
}

void Config::ParseLine( string line ) {
    vector< string > setting;
    string key;
    string value;
    
    line = trim( line );
    if ( line.size() == 0 ) { // empty line
        return;
    }
    if ( line[ 0 ] == '#' ) { // comment
        return;
    }
    setting = explode( " ", line, 2 );
    if ( setting.size() == 2 && setting[ 0 ] == "begin" ) {
        this->MultiSettingBegin( setting[ 1 ] );
        return;
    }
    /* else */ if ( setting.size() == 2 && setting[ 0 ] == "end" ) {
        this->MultiSettingEnd( setting[ 1 ] );
        return;
    }
    if ( this->mCurrentMultiSetting != this->mMultiConf.end() ) {
        this->mCurrentMultiSetting->second.push_back( line );
    }
    else {
        setting = explode( "=", line, 2 );
        if ( setting.size() < 2 ) { // no = sign found
            Trace << "Skipping configuration line; no = sign found";
            return;
        }
        this->SetSetting( setting[ 0 ] , setting[ 1 ] );
    }
}

string Config::GetSetting( string settingname ) {
    map< string, string >::iterator i;

    settingname = strtolower( settingname );
    i = this->mConf.find( settingname );
    if ( i == this->mConf.end() ) {
        return ""; // setting not set
    }
    return i->second;
}

string Config::GetSetting( char* settingname ) {
    string s;
    
    s = settingname;
    return this->GetSetting( s );
}

void Config::SetSetting( string key , string value ) {
    key = strtolower( trim( key ) );
    value = trim( value );
    this->mConf[ key ] = value;
}

void Config::SetSetting( char* key , char* value ) {
    string skey = key;
    string svalue = value;
    
    this->SetSetting( skey , svalue );
}

void Config::SetDefaultSetting( string key , string value ) {
    string setting;

    setting = this->GetSetting( key );
    if ( setting == "" ) {
        this->SetSetting( key , value );
    }
}

void Config::SetDefaultSetting( char* key , char* value ) {
    string skey = key;
    string svalue = value;
    
    this->SetDefaultSetting( skey , svalue );
}

vector< string > Config::GetMultiSetting( char* key ) {
    map< string, vector< string > >::iterator i;

    i = this->mMultiConf.find( key );
    if ( i == this->mMultiConf.end() ) {
        return vector< string >();
    }

    return i->second;
}

void Config::MultiSettingBegin( string key ) {
    this->mCurrentMultiSetting = this->mMultiConf.insert( make_pair( key, vector< string >() ) ).first;
}

void Config::MultiSettingAdd( string value ) {
    this->mCurrentMultiSetting->second.push_back( value );
}

void Config::MultiSettingEnd( string key ) {
    if ( this->mCurrentMultiSetting->first != key ) {
        Warning << "Configuration file " << this->mFilename << " contains incosistent end tag " << key;
    }
    this->mCurrentMultiSetting = this->mMultiConf.end();
}
