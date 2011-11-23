/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : rsaHandler.h

* Purpose :

* Creation Date : 21-11-2011

* Last Modified : Tue 22 Nov 2011 07:07:12 PM EET

* Created By : Greg Liras <gregliras@gmail.com>

_._._._._._._._._._._._._._._._._._._._._.*/
#ifndef RSAHANDLER_H
#define RSAHANDLER_H

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
//#include <stream>
#include <sstream>
#include <iostream>


class rsaHandler// : public Cador_Extension
{
  private:
    RSA *pub_key;
    RSA *pub_remote_key;
    RSA *priv_key;
    long enc_data_length;
    long orig_data_length;
    void get_public_RSA_key();
    unsigned char *encrypted;
    unsigned char *decrypted;
  public:
    rsaHandler();
    ~rsaHandler();
    void set_public_remote_key(RSA *public_remote_key);
    long get_enc_data_length();
    long get_orig_data_length();
    RSA *get_public_key();
    std::string encrypt_data(unsigned char * data);
    std::string decrypt_data(unsigned char * data,long enc_data_length,long data_length);
    void clear_data_memory(); 
};
#endif //RSAHANDLER_H
