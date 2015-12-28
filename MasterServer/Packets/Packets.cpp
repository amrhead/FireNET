/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 14.08.2014   22:09 : Created by AfroStalin(chernecoff)
- 01.06.2015   12:50  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "Packets.h" 
#include <memory>
#include <openssl\blowfish.h>
#include "PacketDebugger.h"


Packet::Packet()
{
	_initNull();
	NEW_BLOWFISH_KEY_LEN = 0;
	memset( (void *)NEW_BLOWFISH_KEY, 0, sizeof(NEW_BLOWFISH_KEY) );
}

Packet::Packet( const unsigned char *bytes, unsigned int length )
{
	_initNull();
	NEW_BLOWFISH_KEY_LEN = 0;
	memset( (void *)NEW_BLOWFISH_KEY, 0, sizeof(NEW_BLOWFISH_KEY) );
	this->setBytes( bytes, length );
}

Packet::~Packet()
{
	_freeSelf();
}

void Packet::_initNull()
{
	BasePacket::_initNull();

	// You super strong key here
	static const unsigned char* statickey = (const unsigned char*)gEnv->pSettings->GetConfigValue("Server","securityKey");
	memcpy(STATIC_BLOWFISH_KEY, statickey, 64);

	/*STATIC_BLOWFISH_KEY[0]  = 0x6B;
	STATIC_BLOWFISH_KEY[1]  = 0x60;
	STATIC_BLOWFISH_KEY[2]  = 0xCB;
	STATIC_BLOWFISH_KEY[3]  = 0x5B;
	STATIC_BLOWFISH_KEY[4]  = 0x82;
	STATIC_BLOWFISH_KEY[5]  = 0xCE;
	STATIC_BLOWFISH_KEY[6]  = 0x90;
	STATIC_BLOWFISH_KEY[7]  = 0xB1;
	STATIC_BLOWFISH_KEY[8]  = 0xCC;
	STATIC_BLOWFISH_KEY[9]  = 0x2B;
	STATIC_BLOWFISH_KEY[10] = 0x6C;
	STATIC_BLOWFISH_KEY[11] = 0x55;
	STATIC_BLOWFISH_KEY[12] = 0x6C;
	STATIC_BLOWFISH_KEY[13] = 0x6C;
	STATIC_BLOWFISH_KEY[14] = 0x6C;
	STATIC_BLOWFISH_KEY[15] = 0x6C;*/
	STATIC_BLOWFISH_KEY_LEN = 64;
	xor_key = 0;
}

bool Packet::decodeBlowfish( bool bUseStaticBFKey )
{
	int blen = (int)getPacketSize();
	int datalen = blen - 2;
	int n8bc = datalen / 8;
	int rest = datalen - n8bc*8;
	if( rest > 0 ) rest = 8;    
	int newdatalen = datalen + rest;
	int newbuflen = newdatalen + 2;
	if( blen < 1 ) return false;
	
	unsigned char *buf = b.getBytesPtr();
	if( !buf ) return false;
	
	BF_KEY bfkey;
	if( bUseStaticBFKey )
		BF_set_key( &bfkey, (int)this->STATIC_BLOWFISH_KEY_LEN,
			this->STATIC_BLOWFISH_KEY );
	else
		BF_set_key( &bfkey, (int)this->NEW_BLOWFISH_KEY_LEN,
			this->NEW_BLOWFISH_KEY );
	
	int offset = 0;
	int nPasses = 0;

	unsigned char outbuf [1024];

/*	if( !outbuf )
	{
		return false;
	}
*/
	memset( outbuf, 0, newbuflen );
	outbuf[0] = buf[0];
	outbuf[1] = buf[1];

	nPasses = 0;
	for( offset=2; offset<newdatalen; offset+=8 )
	{
		unsigned char data[8] = {0,0,0,0,0,0,0,0};
		memcpy( data, buf+offset, 8 );
		BF_decrypt( (BF_LONG *)data, &bfkey );
		memcpy( outbuf+offset, data, 8 );
		nPasses++;
	}	
	this->setBytes( outbuf, blen );
	
	return true;
}

bool Packet::setDynamicBFKey( unsigned char *newKey, unsigned int newKeyLen )
{
	if( !newKey ) return false;
	if( newKeyLen < 1 ) return false;
	memcpy( this->NEW_BLOWFISH_KEY, newKey, newKeyLen );
	this->NEW_BLOWFISH_KEY_LEN = newKeyLen;
	return true;
}

bool Packet::decodeBlowfish( unsigned char *blowfishKey )
{
	setDynamicBFKey( blowfishKey, 16 );
	return decodeBlowfish( false );
}



bool Packet::appendChecksum( bool append4bytes )
{
	unsigned char *raw = b.getBytesPtr();
	int size = (int)this->real_size;
	unsigned int chksum = 0;
	int count = size;
	unsigned int ecx = 0;
	int i = 0;
	int offset = 2;
	for( i = offset; i<count; i+=4 )
	{
		ecx  = (raw[i])           & 0x000000ff;
		ecx |= (raw[i+1] << 0x08) & 0x0000ff00;
		ecx |= (raw[i+2] << 0x10) & 0x00ff0000;
		ecx |= (raw[i+3] << 0x18) & 0xff000000;

		chksum ^= ecx;
	}

	ecx  = raw[i]           & 0x000000ff;
	ecx |= raw[i+1] << 0x08 & 0x0000ff00;
	ecx |= raw[i+2] << 0x10 & 0x00ff0000;
	ecx |= raw[i+3] << 0x18 & 0xff000000;

	writeUChar( (unsigned char)((chksum)         & 0xff) );
	writeUChar( (unsigned char)((chksum >> 0x08) & 0xff) );
	writeUChar( (unsigned char)((chksum >> 0x10) & 0xff) );
	writeUChar( (unsigned char)((chksum >> 0x18) & 0xff) );

	if( append4bytes )
	{
		writeUChar( 0x00 );
		writeUChar( 0x00 );
		writeUChar( 0x00 );
		writeUChar( 0x00 );
	}
	return true;
}

bool Packet::verifyBytesChecksum( const unsigned char *bytes, unsigned int offset, unsigned int size )
{
	if( !bytes || (size<8) ) return false;
	if( size % 4 != 0 ) return false;

	unsigned int chksum = 0;
	unsigned int count = size-4;
	unsigned int check = 0xFFFFFFFF;
	unsigned int i = 0;
	//
	for( i=offset; i<count; i+=4 )
	{
		check = bytes[i] & 0xFF;
		check |= (bytes[i+1] << 8) & 0xFF00;
		check |= (bytes[i+2] << 16) & 0xFF0000;
		check |= (bytes[i+3] << 24) & 0xFF000000;
		chksum ^= check;
	}
	check = bytes[i] & 0xFF;
	check |= (bytes[i+1] << 8) & 0xFF00;
	check |= (bytes[i+2] << 16) & 0xFF0000;
	check |= (bytes[i+3] << 24) & 0xFF000000;
	//
	return (check == chksum);
}

bool Packet::verifyChecksum() const
{
	const unsigned char *bytes = getBytesPtr();
	unsigned int data_len = getDataSize();
	return Packet::verifyBytesChecksum( bytes, 2, data_len );
}

bool Packet::padPacketTo8ByteLen()
{
	unsigned char *packet = b.getBytesPtr();
	unsigned int plen = getPacketSize();
	if( !packet || plen<3 ) return false;
	unsigned int datalen = plen-2;
	
	unsigned int rest = datalen % 8;
	unsigned int addsize = 0;
	if( rest > 0 ) addsize = 8 - rest;
	
	if( addsize > 0 )
	{
		unsigned int i;
		for( i=0; i<addsize; i++ ) this->writeUChar( 0x00 );

	}
	
	return true;
}

bool Packet::appendMore8Bytes()
{
	int i;
	for( i=0; i<8; i++ ) writeUChar( 0x00 );
	return true;
}

bool Packet::encodeBlowfish( bool bUseStaticBFKey )
{
	unsigned char *buf = this->b.getBytesPtr();
	if( !buf ) return false;
	unsigned int blen = getPacketSize();
	if( blen < 1 ) return false;

	BF_KEY bfkey;
	if( bUseStaticBFKey )
		BF_set_key( &bfkey, (int)this->STATIC_BLOWFISH_KEY_LEN,
			this->STATIC_BLOWFISH_KEY );
	else
		BF_set_key( &bfkey, (int)this->NEW_BLOWFISH_KEY_LEN,
			this->NEW_BLOWFISH_KEY );
	
	unsigned int offset = 0;
	int nPasses = 0;

	unsigned char outbuf [1024];

/*	if( !outbuf )
	{
		return false;
	}
*/
	memset( outbuf, 0, blen );
	outbuf[0] = buf[0];
	outbuf[1] = buf[1];

	for( offset=2; offset<real_size-2; offset+=8 )
	{
		unsigned char data[8] = {0,0,0,0,0,0,0,0};
		memcpy( data, buf+offset, 8 );
		BF_encrypt( (BF_LONG *)data, &bfkey );
		memcpy( outbuf+offset, data, 8 );
		nPasses++;
	}	
	this->setBytes( outbuf, blen );

	return true;
}

bool Packet::encodeAndPrepareToSend( unsigned char *blowfishKey, unsigned int bfKeyLen /*= 16*/ )
{
	if( !setDynamicBFKey( blowfishKey, bfKeyLen ) ) return false;
	if( !padPacketTo8ByteLen() ) return false;
	appendChecksum( true );
	appendMore8Bytes();
	return encodeBlowfish( false );
}