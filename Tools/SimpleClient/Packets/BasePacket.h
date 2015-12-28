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

#ifndef _BasePacket_
#define _BasePacket_

#include "ByteArray.h"

class BasePacket
{
public:
	BasePacket();
	BasePacket( const unsigned char *bytes, unsigned int length );
	virtual ~BasePacket();

public:
	virtual unsigned char  getByteAt( unsigned int index );
	virtual unsigned char  setByteAt( unsigned int index, unsigned char byte );
	virtual bool           setBytes( const unsigned char *bytes, unsigned int length );
	virtual bool           setBytesPtr( unsigned char *bytes, unsigned int length );
	virtual void           setPacketType( unsigned char type );
	virtual void           setPacketType2( unsigned char opcode, unsigned short opcode2 );
	virtual void           setPacketType3( unsigned char opcode, unsigned short opcode2, unsigned short opcode3 );
	virtual unsigned char  getPacketType() { readReset(); return readUChar(); }
	virtual unsigned short getPacketSize() const { return (unsigned short)(this->real_size); }
	virtual unsigned short getDataSize() const { return (unsigned short)(this->datasize); }

public: 
	virtual bool           ensureCanWriteBytes( unsigned int nBytes );
	virtual void           writeReset();
	virtual void           writeChar( char c );
	virtual void           writeUChar( unsigned char c );
	virtual void           writeShort( short int s );
	virtual void           writeUShort( unsigned short int s );
	virtual void           writeInt( int i );
	virtual void           writeUInt( unsigned int i );
	virtual void           writeInt64( long long int i64 );
	virtual void           writeUInt64( unsigned long long int i64 );
	virtual void           writeDouble( double d );
	virtual void           writeString( const char *str );
	virtual void           writeUnicodeString( const wchar_t *ustr );
	virtual void           writeBytes( const unsigned char *bytes, unsigned int len );
public:
	virtual bool           canReadBytes( unsigned int nBytes );
	virtual void           readReset();
	virtual char           readChar();
	virtual unsigned char  readUChar();
	virtual short int      readShort();
	virtual unsigned short int readUShort();
	virtual int            readInt();
	virtual unsigned int   readUInt();
	virtual long long int  readInt64();
	virtual unsigned long long int readUInt64();
	virtual double         readDouble();
	virtual char          *readString();
	virtual wchar_t       *readUnicodeString();
	virtual const wchar_t *readUnicodeStringPtr();
	virtual bool           readBytes( unsigned char *bytes, unsigned int num );
public:
	virtual inline void writeC( char c )                 { writeChar( c ); }
	virtual inline void writeH( short h )                { writeShort( h ); }
	virtual inline void writeD( int d )                  { writeInt( d ); }
	virtual inline void writeQ( long long int Q )        { writeInt64( Q ); }
	virtual inline void writeF( double f )               { writeDouble( f ); }
	virtual inline void writeS( const wchar_t *string )  { writeUnicodeString( string ); }
	virtual inline void writeB( const ByteArray& bytes ) { writeBytes( bytes.getBytesPtr(), bytes.getSize() ); }
	virtual inline char           readC() { return readChar(); }
	virtual inline short          readH() { return readShort(); }
	virtual inline int            readD() { return readInt(); }
	virtual inline double         readF() { return readDouble(); }
	virtual inline long long int  readQ() { return readInt64(); }
	virtual inline wchar_t       *readS() { return readUnicodeString(); }
	virtual        ByteArray     *readB( unsigned int count );
public: 
	virtual bool parse();
	virtual bool create();
	
public:
	virtual const unsigned char *getBytesPtr() const;
protected:
	virtual void _initNull();
	virtual bool _preAllocateBuffer();
	virtual bool _growBuffer();
	virtual void _freeSelf();
protected:
	unsigned int buffer_size;
	unsigned int real_size;
	unsigned int write_ptr;
	unsigned int read_ptr;
	unsigned int datasize;
	ByteArray b;
};

#endif