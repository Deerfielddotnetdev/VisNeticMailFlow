// md5class.cpp: implementation of the CMD5 class.
//See internet RFC 1321, "The MD5 Message-Digest Algorithm"
//
//Use this code as you see fit. It is provided "as is"
//without express or implied warranty of any kind.

//////////////////////////////////////////////////////////////////////

#include "md5class.h"
#include "md5.h" //declarations from RFC 1321
#include <string.h>	  
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMD5::CMD5()
{
	m_digestValid = false; //we don't have a plaintext string yet
	m_digestString[32]=0;  //the digest string is always 32 characters, so put a null in position 32
}

CMD5::~CMD5()
{

}

CMD5::CMD5(const char* plainText)  
{	
	m_plainText =  const_cast<char*>(plainText); //get a pointer to the plain text.  If casting away the const-ness worries you,
												 //you could make a local copy of the plain text string.
   	m_digestString[32]=0;
	m_digestValid = calcDigest();
}

//////////////////////////////////////////////////////////////////////
// Implementation
//////////////////////////////////////////////////////////////////////

void CMD5::setPlainText(const char* plainText)
{
  //set plaintext with a mutator, it's ok to 
  //to call this multiple times.  If casting away the const-ness of plainText 
  //worries you, you could either make a local copy of the plain 
  //text string instead of just pointing at the user's string, or 
  //modify the RFC 1321 code to take 'const' plaintext, see example below. 

  m_plainText = const_cast<char*>(plainText);
  m_digestValid = calcDigest();
}

/* Use a function of this type with your favorite string class
   if casting away the const-ness of the user's text buffer violates you
   coding standards. 

void CMD5::setPlainText(CString& strPlainText)
{
	static CString plaintext(strPlainText);
	m_plainText = strPlainText.GetBuffer();
	m_digestValid = calcDigest();
}
*/

const char* CMD5::getMD5Digest()
 {	//access message digest (aka hash), return 0 if plaintext has not been set
	if(m_digestValid)
	{
		return m_digestString;
	} else return 0;
 }




bool CMD5::calcDigest()
{
  //See RFC 1321 for details on how MD5Init, MD5Update, and MD5Final 
  //calculate a digest for the plain text
  MD5_CTX context;
  MD5Init(&context); 

  //the alternative to these ugly casts is to go into the RFC code and change the declarations
  MD5Update(&context, reinterpret_cast<unsigned char *>(m_plainText), ::strlen(m_plainText));
  MD5Final(reinterpret_cast <unsigned char *>(m_digest),&context);
  
  //make a string version of the numeric digest value
  int p=0;
  for (int i = 0; i<16; i++)
  {
	::sprintf(&m_digestString[p],"%02x", m_digest[i]);
	p+=2;
  }
  return true;
}

// important: this function must be used separately from the other CMD5
// functions (and constructor); I will work toward better class integration
// when time permits - MER
void CMD5::GetKeyedDigest(unsigned char* text, int text_len, unsigned char* key, int key_len, unsigned char* digest)
{
	unsigned char tk[16];
	
	// if key is longer than 64 bytes reset it to key=MD5(key)
	if (key_len > 64) 
	{
		MD5_CTX tctx;
		MD5Init(&tctx);
		MD5Update(&tctx, key, key_len);
		MD5Final(tk, &tctx);
		
		key = tk;
		key_len = 16;
	}
	
	// the HMAC_MD5 transform looks like:
	//
	// MD5(K XOR opad, MD5(K XOR ipad, text))
	//
	// where K is an n byte key
	// ipad is the byte 0x36 repeated 64 times
	// opad is the byte 0x5c repeated 64 times
	// and text is the data being protected
	
	//start out by storing key in pads
	unsigned char k_ipad[65];    // inner padding - key XORd with ipad
	memset(k_ipad, 0, 64);
	unsigned char k_opad[65];    // outer padding - key XORd with opad
	memset(k_opad, 0, 64);
	memcpy(k_ipad, key, key_len);
	memcpy(k_opad, key, key_len);
	
	// XOR key with ipad and opad values
	for (int i=0; i<64; i++) 
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	
	//perform inner MD5
	MD5_CTX context;
	MD5Init(&context);                   // init context for 1st pass
	MD5Update(&context, k_ipad, 64);     // start with inner pad
	MD5Update(&context, text, text_len); // then text of datagram
	MD5Final(digest, &context);          // finish up 1st pass
	
	//perform outer MD5
	MD5Init(&context);                   // init context for 2nd pass
	MD5Update(&context, k_opad, 64);     // start with outer pad
	MD5Update(&context, digest, 16);     // then results of 1st hash
	MD5Final(digest, &context);          // finish up 2nd pass
}



