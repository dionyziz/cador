/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

* File Name : rsaHandler.h

* Purpose :

* Creation Date : 21-11-2011

* Last Modified : Mon 21 Nov 2011 02:35:56 PM EET

* Created By : Greg Liras <gregliras@gmail.com>

_._._._._._._._._._._._._._._._._._._._._.*/
#ifndef RSAHANDLER_H
#define RSAHANDLER_H

#include "../extension.h"
#include "../conf.h"
#include "../error.h"

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <stream>
#include <sstream>


class rsaHandler() : public Cador_Extension
{
  private:
    RSA *pub_key;
    RSA *pub_remote_key;
    RSA *priv_key;
    long enc_message_length;
    std::string encrypt_data(std::string data);
    std::string decrypr_data(std::string data);
  public:
    rsaHandler();
    ~rsaHandler();
    void generate_RSA_key(std::string pass);
    void send_encrypted_data(std::string data);
}
#endif //RSAHANDLER_H