/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
--------------------------------------------------------------------------
History:

- 14.08.2014   22:09 : Created by AfroStalin(chernecoff)
- 07.03.2015   02:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _ByteArray_
#define _ByteArray_

class ByteArray
{
public:
	ByteArray();
	ByteArray( unsigned int length );
	ByteArray( const char *string );
	ByteArray( const ByteArray& ba );
	ByteArray( const unsigned char *newBytes, unsigned int length );
	~ByteArray();

public:
	inline unsigned int   getSize() const { return this->byteCount; }
	bool                  setSize( unsigned int newSize );

public:
	inline unsigned char *getBytesPtr() const { return this->bytes; }
	unsigned char         getByteAt( unsigned int index ) const;
	unsigned char         setByteAt( unsigned int index, unsigned char byteSet );
	void                  memset( unsigned char c );
	bool                  setBytes( const unsigned char *newBytes, unsigned int length );
	bool                  setBytesFromString( const char *string );
	bool                  setBytesFromPtrNoMemcpy( unsigned char *bytes, unsigned int length );

public:
	unsigned char operator[]( int index ) const;
protected:
	void                  _initNull();
	void                  _freeSelf();
protected:
	unsigned int   byteCount;
	unsigned char *bytes;
};

#endif