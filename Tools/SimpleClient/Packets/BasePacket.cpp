/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

Description: Базовая структура всех пакетов
Description: The basic structure of all packages
-------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 07.03.2015   02:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "BasePacket.h"
#include <memory>

BasePacket::BasePacket()
{
	this->_initNull();
}

BasePacket::BasePacket( const unsigned char *bytes, unsigned int length )
{
	this->_initNull();
	this->setBytes( bytes, length );
}

BasePacket::~BasePacket()
{
	_freeSelf();
}

bool BasePacket::setBytes( const unsigned char *bytes, unsigned int length )
{
	_initNull();
	if( !bytes || (length < 2) ) return false;
	this->ensureCanWriteBytes( length );
	bool ret = this->b.setBytes( bytes, length );
	if( ret )
	{
		this->buffer_size = b.getSize();
		this->real_size = length;
		this->datasize  = length - 2; 
		this->write_ptr = length;
		this->read_ptr  = 2;
		this->b.setByteAt( 0, (unsigned char)((length)      & 0xff) );
		this->b.setByteAt( 1, (unsigned char)((length >> 8) & 0xff) );
	}
	return ret;
}

bool BasePacket::setBytesPtr( unsigned char *bytes, unsigned int length )
{
	if( !bytes || (length < 2) ) return false;
	if( this->b.setBytesFromPtrNoMemcpy( bytes, length ) )
	{
		this->buffer_size = b.getSize();
		this->real_size = length;
		this->datasize = length - 2;
		this->write_ptr = length;
		this->read_ptr = 2;
		this->b.setByteAt( 0, (unsigned char)((length)      & 0xff) );
		this->b.setByteAt( 1, (unsigned char)((length >> 8) & 0xff) );
		return true;
	}
	return false;
}

void BasePacket::setPacketType( unsigned char type )
{
	writeReset();
	writeUChar( type );
}

void BasePacket::setPacketType2( unsigned char opcode, unsigned short opcode2 )
{
	writeReset();
	writeUChar( opcode );
	writeUShort( opcode2 );
}

void BasePacket::setPacketType3( unsigned char opcode, unsigned short opcode2, unsigned short opcode3 )
{
	writeReset();
	writeUChar( opcode );
	writeUShort( opcode2 );
	writeUShort( opcode3 );
}

bool BasePacket::ensureCanWriteBytes( unsigned int nBytes )
{
	if( nBytes == 0 ) return true;
	unsigned int end_ptr = write_ptr + nBytes - 1;

	while( end_ptr >= buffer_size )
	{
		if( !this->_growBuffer() ) return false;
	}
	return true;
}

void BasePacket::writeChar( char c )
{
	if( !this->ensureCanWriteBytes( 1 ) ) return;
	this->b.setByteAt( this->write_ptr, (unsigned char)c );
	this->write_ptr++;
	this->real_size++;
	this->datasize++;

	unsigned short psize = (unsigned short)real_size;
	unsigned char *psaddr = (unsigned char *)&psize;
	b.setByteAt( 0, psaddr[0] );
	b.setByteAt( 1, psaddr[1] );
}

void BasePacket::writeUChar( unsigned char c )
{
	if( !this->ensureCanWriteBytes( 1 ) ) return;
	this->b.setByteAt( this->write_ptr, c );
	this->write_ptr++;
	this->real_size++;
	this->datasize++;

	unsigned short psize = (unsigned short)real_size;
	unsigned char *psaddr = (unsigned char *)&psize;
	b.setByteAt( 0, psaddr[0] );
	b.setByteAt( 1, psaddr[1] );
}

void BasePacket::writeBytes( const unsigned char *bytes, unsigned int len )
{
	if( !bytes || (len<1) ) return;
	unsigned int i;
	for( i=0; i<len; i++ ) this->writeUChar( bytes[i] );
}

void BasePacket::writeShort( short int s )
{
	char leftByte = (char)(s & 0x00FF);
	char rightByte = (char)( (s & 0xFF00) >> 8 );
	this->writeChar( leftByte );
	this->writeChar( rightByte );
}

void BasePacket::writeUShort( unsigned short int s )
{
	unsigned char leftByte  = (unsigned char)(  s & (unsigned short)0x00FF);
	unsigned char rightByte = (unsigned char)( (s & (unsigned short)0xFF00) >> 8 );
	this->writeUChar( leftByte );
	this->writeUChar( rightByte );
}

void BasePacket::writeInt( int i )
{
	char b1 = (char)(  i & (int)0x000000FF);
	char b2 = (char)( (i & (int)0x0000FF00) >> 8 );
	char b3 = (char)( (i & (int)0x00FF0000) >> 16 );
	char b4 = (char)( (i & (int)0xFF000000) >> 24 );
	this->writeChar( b1 );
	this->writeChar( b2 );
	this->writeChar( b3 );
	this->writeChar( b4 );
}

void BasePacket::writeUInt( unsigned int i )
{
	unsigned char b1 = (unsigned char)(  i & (unsigned int)0x000000FF);
	unsigned char b2 = (unsigned char)( (i & (unsigned int)0x0000FF00) >> 8 );
	unsigned char b3 = (unsigned char)( (i & (unsigned int)0x00FF0000) >> 16 );
	unsigned char b4 = (unsigned char)( (i & (unsigned int)0xFF000000) >> 24 );
	this->writeUChar( b1 );
	this->writeUChar( b2 );
	this->writeUChar( b3 );
	this->writeUChar( b4 );
}

void BasePacket::writeInt64( long long int i64 )
{
	char *i64b = (char *)(&i64);
	unsigned int i;
	for( i=0; i<sizeof(long long int); i++ ) this->writeChar( i64b[i] );
}

void BasePacket::writeUInt64( unsigned long long int i64 )
{
	unsigned char *i64b = (unsigned char *)(&i64);
	unsigned int i;
	for( i=0; i<sizeof(unsigned long long int); i++ ) this->writeUChar( i64b[i] );
}

void BasePacket::writeDouble( double d )
{
	char *pd = (char *)(&d);
	unsigned int i;
	for( i=0; i<sizeof(double); i++ ) this->writeChar( pd[i] );
}

void BasePacket::writeString( const char *str )
{
	if( !str ) return;
	char *p = (char *)str;
	while( (*p) )
	{
		this->writeChar( (*p) );
		p++;
	}
	this->writeChar( 0 );
}

void BasePacket::writeUnicodeString( const wchar_t *ustr )
{
	if( !ustr ) return;
	wchar_t *p = (wchar_t *)ustr;
	while( (*p) )
	{
		this->writeShort( (*p) );
		p++;
	}
	this->writeShort( 0x0000 );
}

bool BasePacket::canReadBytes( unsigned int nBytes )
{
	if( nBytes == 0 ) return true;
	unsigned int end_ptr = read_ptr + nBytes;
	if( end_ptr <= real_size ) return true;
	return false;
}

char BasePacket::readChar()
{
	if( !canReadBytes( 1 ) )
		return 0;
	char ret = b.getByteAt( read_ptr );
	read_ptr++;
	return ret;
}

unsigned char BasePacket::readUChar()
{
	if( !canReadBytes( 1 ) )
		return 0;

	unsigned char ret = b.getByteAt( read_ptr );
	read_ptr++;
	return ret;
}

bool BasePacket::readBytes( unsigned char *bytes, unsigned int num )
{
	if( !bytes || (num<1) ) return false;
	unsigned int i;
	for( i=0; i<num; i++ ) bytes[i] = this->readUChar();
	return true;
}

short int BasePacket::readShort()
{
	if( canReadBytes( 2 ) )
	{
		unsigned char c1 = readChar();
		unsigned char c2 = readChar();
		unsigned short int ret = ((unsigned short int)c1) | ((unsigned short int)c2 << 8);
		return ret;
	}
	return 0;
}

unsigned short int BasePacket::readUShort()
{
	if( canReadBytes( 2 ) )
	{
		unsigned char c1 = readUChar();
		unsigned char c2 = readUChar();
		unsigned short int ret = ((unsigned short int)c1) | ((unsigned short int)c2 << 8);
		return ret;
	}
	return 0;
}

int BasePacket::readInt()
{
	if( canReadBytes( 4 ) )
	{
		unsigned char c1 = readChar();
		unsigned char c2 = readChar();
		unsigned char c3 = readChar();
		unsigned char c4 = readChar();
		unsigned int ret = (unsigned int)c1;
		ret |= ( (unsigned int)c2 << 8 );
		ret |= ( (unsigned int)c3 << 16 );
		ret |= ( (unsigned int)c4 << 24 );
		return ret;
	}
	return 0;
}

unsigned int BasePacket::readUInt()
{
	if( canReadBytes( 4 ) )
	{
		unsigned char c1 = readChar();
		unsigned char c2 = readChar();
		unsigned char c3 = readChar();
		unsigned char c4 = readChar();
		unsigned int ret = (unsigned int)c1;
		ret |= ( (unsigned int)c2 << 8 );
		ret |= ( (unsigned int)c3 << 16 );
		ret |= ( (unsigned int)c4 << 24 );
		return ret;
	}
	return 0;
}

long long int BasePacket::readInt64()
{
	long long int ret = 0;
	if( this->canReadBytes( 8 ) )
	{
		unsigned int i;
		unsigned char c = 0;
		unsigned char *retaddr = (unsigned char *)&ret;
		for( i=0; i<sizeof(long long int); i++ )
		{
			c = readChar();
			(*retaddr) = c;
			retaddr++;
		}
		return ret;
	}
	return 0;
}

unsigned long long int BasePacket::readUInt64()
{
	unsigned long long int ret = 0;
	if( this->canReadBytes( 8 ) )
	{
		unsigned int i;
		unsigned char c = 0;
		unsigned char *retaddr = (unsigned char *)&ret;
		for( i=0; i<sizeof(unsigned long long int); i++ )
		{
			c = readUChar();
			(*retaddr) = c;
			retaddr++;
		}
		return ret;
	}
	return 0;
}

double BasePacket::readDouble()
{
	double ret = 0.0;
	if( canReadBytes( 8 ) )
	{
		unsigned int i = 0;
		unsigned char c = 0;
		unsigned char *retaddr = (unsigned char *)&ret;
		for( i=0; i<sizeof(double); i++ )
		{
			c = readChar();
			(*retaddr) = c;
			retaddr++;
		}
		return ret;
	}
	return 0.0;
}

char *BasePacket::readString()
{
	unsigned int save_read_ptr = this->read_ptr;
	unsigned int str_len = 0;
	char c = 0;
	char *str = NULL;
	while( this->canReadBytes(1) )
	{
		c = this->readChar();
		str_len++;
		if( c == 0 ) break;
	}
	this->read_ptr = save_read_ptr;
	if( str_len == 0 ) return NULL;
	str = (char *)malloc( str_len+1 );
	if( !str ) return NULL;
	str[str_len] = 0x00;
	if( this->readBytes( (unsigned char *)str, str_len ) ) return str;
	free( str );
	str = NULL;
	return NULL;
}

wchar_t *BasePacket::readUnicodeString()
{
	unsigned int save_read_ptr = this->read_ptr;
	unsigned int str_len = 0;
	unsigned int buffer_len = 0;
	unsigned short int c = 0;
	wchar_t *wstr = NULL;
	while( this->canReadBytes(2) )
	{
		c = this->readUShort();
		str_len++;
		buffer_len += 2;
		if( c == 0x0000 ) break;
	}
	this->read_ptr = save_read_ptr;
	if( str_len == 0 ) return NULL;
	wstr = (wchar_t *)malloc( buffer_len+2 );
	if( !wstr ) return NULL;
	wstr[str_len] = 0x0000;
	if( this->readBytes( (unsigned char *)wstr, buffer_len ) ) return wstr;
	free( wstr );
	wstr = NULL;
	return NULL;
}

const wchar_t *BasePacket::readUnicodeStringPtr()
{
	unsigned short int ch = 0;
	wchar_t *wstr = (wchar_t *)( this->getBytesPtr() + this->read_ptr );

	while( this->canReadBytes(2) )
	{
		ch = this->readUShort();
		if( ch == 0x0000 ) break;
	}
	return wstr;
}


void BasePacket::writeReset()
{
	write_ptr = 2; 
	real_size = 2;
	read_ptr  = 2; 
	datasize  = 0;
	_preAllocateBuffer();
	b.setByteAt( 0, 2 );
	b.setByteAt( 1, 0 );
}

void BasePacket::readReset()
{
	read_ptr = 2;
}

bool BasePacket::_preAllocateBuffer()
{
	if( !b.setSize( buffer_size ) ) return false;
	return true;
}

bool BasePacket::_growBuffer()
{
	buffer_size *= 2;
	return _preAllocateBuffer();
}

void BasePacket::_initNull()
{
	buffer_size = 256;
	datasize = 0;
	writeReset();
	readReset();
}

void BasePacket::_freeSelf()
{
	buffer_size = 256;
	writeReset();
	readReset();
}

unsigned char BasePacket::getByteAt( unsigned int index )
{
	if( index >= real_size )
		return 0;
	char c = b.getByteAt( index );
	return c;
}
unsigned char BasePacket::setByteAt( unsigned int index, unsigned char byte )
{
	return b.setByteAt( index, byte );
}

inline const unsigned char *BasePacket::getBytesPtr() const
{
	return b.getBytesPtr();
}

ByteArray *BasePacket::readB( unsigned int count )
{
	ByteArray *pByteArray = new ByteArray();
//	if( !pByteArray ) return NULL;
	pByteArray->setSize( count );
	if( this->readBytes( pByteArray->getBytesPtr(), count ) ) return pByteArray;
	delete pByteArray;
	return NULL;
}

bool BasePacket::parse()
{
	this->readReset();
	this->readUChar();
	return true;
}

bool BasePacket::create()
{
	this->writeReset();
	return true;
}

