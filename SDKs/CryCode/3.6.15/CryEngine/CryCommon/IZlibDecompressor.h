////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2012
// ------------------------------------------------------------------------
//  File name:   IZlibDecompressor.h
//  Created:     30/8/2012 by Axel Gneiting
//  Description: Provides the interface for the zlib inflate wrapper
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IZLIBDECOMPRESSOR_H__
#define __IZLIBDECOMPRESSOR_H__

enum EZInflateState
{
	eZInfState_AwaitingInput,		// caller must call Input() to continue
	eZInfState_Inflating,				// caller must wait
	eZInfState_ConsumeOutput,		// caller must consume output and then call SetOutputBuffer() to continue	
	eZInfState_Finished,				// caller must call Release()
	eZInfState_Error						// error has occurred and the stream has been closed and will no longer compress
};

UNIQUE_IFACE struct IZLibInflateStream
{
protected:
	virtual ~IZLibInflateStream() {}; // use Release()

public:
	struct SStats
	{
		int bytesInput;
		int bytesOutput;
		int curMemoryUsed;
		int peakMemoryUsed;
	};

	// Description:
	//	 Specifies the output buffer for the inflate operation
	//	 Should be set before providing input
	//	 The specified buffer must remain valid (ie do not free) whilst compression is in progress (state == eZInfState_Inflating)	
	virtual void SetOutputBuffer(char *pInBuffer, unsigned int inSize) = 0;

	// Description:
	//	 Returns the number of bytes from the output buffer that are ready to be consumed. After consuming any output, you should call SetOutputBuffer() again to mark the buffer as available
	virtual unsigned int GetBytesOutput() = 0;

	// Description:
	//	 Begins decompressing the source data pInSource of length inSourceSize to a previously specified output buffer
	//	 Only valid to be called if the stream is in state eZInfState_AwaitingInput
	//	 The specified buffer must remain valid (ie do not free) whilst compression is in progress (state == eZInfState_Inflating)
	virtual void Input(const char *pInSource, unsigned int inSourceSize) = 0;

	// Description:
	//	 Finishes the compression, causing all data to be flushed to the output buffer
	//	 Once called no more data can be input
	//	 After calling the caller must wait until GetState() reuturns eZInfState_Finished
	virtual void EndInput() = 0;

	// Description:
	//	 Returns the state of the stream,
	virtual EZInflateState GetState() = 0;

	// Description:
	//	 Gets stats on inflate stream, valid to call at anytime
	virtual void GetStats(SStats *pOutStats) = 0;

	// Description:
	//	 Deletes the inflate stream. Will assert if stream is in an invalid state to be released (in state eZInfState_Inflating)
	virtual void Release() = 0;
};

UNIQUE_IFACE struct IZLibDecompressor
{
	protected:
		virtual ~IZLibDecompressor()	{}; // use Release()

	public:
		// Description:
		//	 Creates a inflate stream to decompress data using zlib
		virtual IZLibInflateStream *CreateInflateStream() = 0;

		virtual void Release() = 0;
};

#endif // __IZLIBCOMPRESSOR_H__

