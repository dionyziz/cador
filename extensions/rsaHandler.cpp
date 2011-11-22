/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

 * File Name : rsaHandler.cpp

 * Purpose :

 * Creation Date : 21-11-2011

 * Last Modified : Tue 22 Nov 2011 07:07:39 PM EET

 * Created By : Greg Liras <gregliras@gmail.com>

 _._._._._._._._._._._._._._._._._._._._._.*/
#include "rsaHandler.h"
using namespace std;
rsaHandler::rsaHandler()
{

  int num = 2048;   //needs to be > 1024 to be considered secure;
  int e = 17;    //exponent
  //RSA_ADD(buf,num,entropy); //not needed since we have /dev/urandom
  //pub_key = RSA_generate_key(num,e,NULL,NULL);
  //if (pub_key == NULL)
  //{
  //  cout << "Failed to initialize pub_key";
  //}
  //RSA_ADD(buf,num,entropy); //not needed since we have /dev/urandom
  priv_key = RSA_new();
  priv_key = RSA_generate_key(num,e,NULL,NULL);
  if (priv_key == NULL)
  {
    cout << "Failed to initialize priv_key";
  }
  pub_key = RSA_new();
  get_public_RSA_key();

  enc_data_length = 0;
  orig_data_length = 0;

  pub_remote_key = NULL;
  encrypted = NULL;
  decrypted = NULL;
  pub_remote_key = NULL;
}
rsaHandler::~rsaHandler()
{
  RSA_free(priv_key);
  RSA_free(pub_key);
}
RSA *rsaHandler::get_public_key()
{
  return this->pub_key;
}
void rsaHandler::set_public_remote_key(RSA *public_remote_key)
{
  cout << "OK got the key" << endl;
  this->pub_remote_key = public_remote_key;
}
string rsaHandler::encrypt_data(unsigned char * data)
{
  this->orig_data_length = strlen((const char *) data);
  if(this->encrypted != NULL)
  {
    clear_data_memory();
  }
  this->encrypted =  (unsigned char *)malloc(RSA_size(this->pub_remote_key));
  this->enc_data_length = RSA_public_encrypt(this->orig_data_length, (unsigned char *) data, this->encrypted, this->pub_remote_key, RSA_PKCS1_PADDING);
  if (enc_data_length == -1)
  {
    cout << "RSA encryption failed" << endl;
  }
  return string((const char *)this->encrypted);
}
string rsaHandler::decrypt_data(unsigned char* data,long enc_data_length,long orig_data_length)
{
  this->orig_data_length = orig_data_length;
  this->enc_data_length = enc_data_length;
  if(this->decrypted != NULL)
  {
    clear_data_memory();
  }
  this->decrypted =  (unsigned char *)malloc(this->orig_data_length);
  int n = RSA_private_decrypt(this->enc_data_length, data, this->decrypted, priv_key, RSA_PKCS1_PADDING);
  if(n == -1)
  {
    cout << "RSA decryption failed " << n << endl;
  }
  return string((const char *)this->decrypted);
}
long rsaHandler::get_enc_data_length()
{
  return this->enc_data_length;
}
long rsaHandler::get_orig_data_length()
{
  return this->orig_data_length;
}
void rsaHandler::get_public_RSA_key()
{
  BIO *bio_buffer = BIO_new(BIO_s_mem());
  PEM_write_bio_RSAPublicKey(bio_buffer,priv_key);
  PEM_read_bio_RSAPublicKey(bio_buffer,&pub_key,NULL,NULL);
  BIO_free(bio_buffer);
}
void rsaHandler::clear_data_memory()
{
  free(encrypted);
  free(decrypted);
}
