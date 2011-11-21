/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

 * File Name : rsaHandler.cpp

 * Purpose :

 * Creation Date : 21-11-2011

 * Last Modified : Mon 21 Nov 2011 03:54:24 PM EET

 * Created By : Greg Liras <gregliras@gmail.com>

 _._._._._._._._._._._._._._._._._._._._._.*/
#include "rsaHandler.h"

rsaHandler::rsaHandler()
{

  int num = 8192;   //needs to be > 1024 to be considered secure;
  int e = 65537;    //exponent
  //RSA_ADD(buf,num,entropy); //not needed since we have /dev/urandom
  pub_key = RSA_generate_key(num,e,NULL,NULL);
  if (pub_key == NULL)
  {
    Warning << "Failed to initialize pub_key";
  }
  //RSA_ADD(buf,num,entropy); //not needed since we have /dev/urandom
  priv_key = RSA_generate_key(num,e,NULL,NULL);
  if (priv_key == NULL)
  {
    Warning << "Failed to initialize priv_key";
  }
}
rsaHandler::~rsaHandler()
{
  RSA_free(priv_key);
  RSA_free(pub_key);
  RSA_free(pub_remote_key);
}
RSA *rsaHandler::get_public_key()
{
  return this->pub_key;
}
void rsaHandler::set_public_remote_key(RSA *public_remote_key)
{
  this->pub_remote_key = public_remote_key;
}
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
