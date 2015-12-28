/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 14.08.2014   22:09 : Created by AfroStalin(chernecoff)
- 08.03.2015   16:37 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include <stdio.h>
#include <memory>
#include "ByteArray.h"

#pragma warning(disable : 4267)

void ByteArray::_initNull()
{
	this->byteCount = 0;
	this->bytes = 0;
}

void ByteArray::_freeSelf()
{
	if( (this->byteCount > 0) && (this->bytes) )
	{
		free( this->bytes );
		this->bytes = NULL;
		this->byteCount = 0;
	}
}

ByteArray::ByteArray()
{
	this->_initNull();
}

ByteArray::ByteArray( unsigned int length )
{
	this->_initNull();
	if( length > 0 ) this->setSize( length );
}

ByteArray::ByteArray( const char *string )
{
	this->_initNull();
	if( string ) this->setBytesFromString( string );
}

ByteArray::ByteArray( const unsigned char *newBytes, unsigned int length )
{
	this->_initNull();
	this->setBytes( newBytes, length );
}

ByteArray::ByteArray( const ByteArray& ba )
{
	this->_initNull();
	this->setBytes( ba.bytes, ba.byteCount );
}

ByteArray::~ByteArray()
{
	this->_freeSelf();
}

bool ByteArray::setSize( unsigned int newSize )
{
	if( newSize == 0 )
	{
		this->_freeSelf();
		return true;
	}
	if( newSize == this->byteCount ) return true;
	if( newSize > 0 )
	{
		unsigned char *newPtr = (unsigned char *)realloc( this->bytes, newSize );
		if( newPtr )
		{
			this->bytes = newPtr;
			this->byteCount = newSize;
			return true;
		}
		return false;
	}
	return false;
}


unsigned char ByteArray::getByteAt( unsigned int index ) const
{
	if( (this->byteCount > 0) && (this->bytes) )
	{
		if( index < (this->byteCount) ) return this->bytes[index];
		return 0;
	}
	return 0;
}

unsigned char ByteArray::setByteAt( unsigned int index, unsigned char byteSet )
{
	if( (this->byteCount > 0) && (this->bytes) )
	{
		unsigned char retVal = 0;
		if( index < (this->byteCount) )
		{
			retVal = this->bytes[index];
			this->bytes[index] = byteSet;
		}
		return retVal;
	}
	else
	{
		return 0;
	}
}

void ByteArray::memset( unsigned char c )
{
	if( (this->byteCount > 0) && (this->bytes) )
	{
		::memset( this->bytes, c, this->byteCount );
	}
}

bool ByteArray::setBytes( const unsigned char *newBytes, unsigned int length )
{
	if( length < 1 ) return false;
	if( this->byteCount < length )
	{
		if( !this->setSize( length ) ) return false;
	}
	this->memset( 0x00 );
	memcpy( this->bytes, newBytes, length );
	return true;
}

bool ByteArray::setBytesFromString( const char *string )
{
	if( !string ) return false;
	unsigned int str_len = strlen( string );
	if( str_len < 1 ) return false;
	return this->setBytes( (const unsigned char *)string, str_len );
}

bool ByteArray::setBytesFromPtrNoMemcpy( unsigned char *bytes, unsigned int length )
{
	if( !bytes || (length<1) ) return false;
	this->byteCount = length;
	this->bytes = bytes;
	return true;
}

unsigned char ByteArray::operator[]( int index ) const
{
	if( (this->byteCount > 0) && (this->bytes) )
	{
		if( (index>=0) && (index<((int)(this->byteCount))) ) return this->bytes[index];
		return 0;
	}
	return 0;
}
