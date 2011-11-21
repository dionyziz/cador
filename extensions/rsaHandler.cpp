/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

 * File Name : rsaHandler.cpp

 * Purpose :

 * Creation Date : 21-11-2011

 * Last Modified : Mon 21 Nov 2011 02:07:56 PM EET

 * Created By : Greg Liras <gregliras@gmail.com>

 _._._._._._._._._._._._._._._._._._._._._.*/
#include "rsaHandler.h"

std::string rsaHandler::encrypt_data(std::string data)
{
  unsigned char *encrypted =  (unsigned char *)malloc(RSA_size(this->pub_remote_key));
  this->rsa_padding = RSA_public_encrypt(data.size(), (unsigned char *) data.c_str(), encrypted, this->pub_remote_key, RSA_PKCS1_PADDING);
  if (rsa_padding == -1)
  {
    Warning << "RSA encryption failed"
  }
  sstream enc_data;
  enc_data << *encrypted;
  free(encrypted);
  return enc_data.str();
}
std::string rsaHandler::decrypr_data(std::string data)
{
  unsigned char *decrypted =  (unsigned char *)malloc(this->rsa_padding);
  int n = RSA_private_decrypt(this->enc_message_length, data.c_str(), decrypted, priv_key, RSA_PKCS1_PADDING);
  if(n == -1)
  {
    Warning << "RSA decryption failed"
  }
  sstream dec_data;
  dec_data << *decrypted;
  free(decrypted);
  return dec_data.str();
}
