/******************************************************************************
** IZLibCompressor.h
** 23/6/10
******************************************************************************/

#ifndef __IZLIBCOMPRESSOR_H__
#define __IZLIBCOMPRESSOR_H__

/*
	wrapper interface for the zlib compression / deflate interface

	supports multiple compression streams with an async compatible wrapper

	Gotchas:
		the ptr to the input data must remain valid whilst the stream is deflating
		the ptr to the output buffer must remain valid whilst the stream is deflating
		
	****************************************************************************************

	usage example:

	IZLibCompressor			*pComp=GetISystem()->GetIZLibCompressor();
	// see deflateInit2() documentation zlib manual for more info on the parameters here
	// this initializes the stream to produce a gzip format block with fairly low memory requirements
	IZLibDeflateStream	*pStream=pComp->CreateDeflateStream(2,eZMeth_Deflated,24,3,eZStrat_Default,eZFlush_NoFlush);
	char								*pOutput=new char[512];		// arbitrary size
	const char					*pInputData="This is an example piece of data that is to be compressed. It can be any arbitrary block of binary data - not just text";
	const int						inputBlockSize=16;				// to simulate streaming of input, this example provides the input in 16 byte blocks
	int									totalInput=sizeof(pInputData);
	int									bytesInput=0;
	bool								done=false;
	FILE								*outputFile=fopen("myfile.gz","rb");

	do
	{
		EZDeflateState		state=pStream->GetState();

		switch (state)
		{
			case eZDefState_AwaitingInput:
				// 'stream' input data, there is no restriction on the block size you can input, if all the data is available immediately, input all of it at once
				{
					int					inputSize=min(inputBlockSize,totalInput-bytesInput);

					if (inputSize<=0)
					{
						pStream->EndInput();
					}
					else
					{
						pStream->Input(pInputData+bytesInput,inputSize);
						bytesInput+=inputSize;
					}
				}
				break;

			case eZDefState_Deflating:
				// do something more interesting... like getting out of this loop and running the rest of your game...
				break;

			case eZDefState_ConsumeOutput:
				// stream output to a file
				{
					int		bytesToOutput=pStream->GetBytesOutput();

					if (bytesToOutput>0)
					{
						fwrite(pOutput,1,bytesToOutput,outputFile);
					}

					pStream->SetOutputBuffer(pOutput,sizeof(pOutput));
				}
				break;

			case eZDefState_Finished:
			case ezDefState_Error:
				done=true;
				break;
		}

	} while (!done);

	fclose(outputFile);

	pStream->Release();
	delete [] pOutput;

****************************************************************************************/

// don't change the order of these zlib wrapping enum values without updating the mapping
// implementation in CZLibCompressorStream
enum EZLibStrategy
{
	eZStrat_Default,						// Z_DEFAULT_STRATEGY
	eZStrat_Filtered,						// Z_FILTERED
	eZStrat_HuffmanOnly,				// Z_HUFFMAN_ONLY
	eZStrat_RLE									// Z_RLE
};
enum EZLibMethod
{
	eZMeth_Deflated							// Z_DEFLATED
};
enum EZLibFlush
{
	eZFlush_NoFlush,						// Z_NO_FLUSH
	eZFlush_PartialFlush,				// Z_PARTIAL_FLUSH
	eZFlush_SyncFlush,					// Z_SYNC_FLUSH
	eZFlush_FullFlush,					// Z_FULL_FLUSH
};

enum EZDeflateState
{
	eZDefState_AwaitingInput,		// caller must call Input() or Finish() to continue
	eZDefState_Deflating,				// caller must wait
	eZDefState_ConsumeOutput,		// caller must consume output and then call SetOutputBuffer() to continue
	eZDefState_Finished,				// stream finished, caller must call Release() to destroy stream
	eZDefState_Error						// error has occurred and the stream has been closed and will no longer compress
};

struct IZLibDeflateStream
{
	protected:
		virtual												~IZLibDeflateStream() {};				// use Release()

	public:
		struct SStats
		{
			int		bytesInput;
			int		bytesOutput;
			int		curMemoryUsed;
			int		peakMemoryUsed;
		};

		// <interfuscator:shuffle>
		// Description:
		//	 Specifies the output buffer for the deflate operation
		//	 Should be set before providing input
		//	 The specified buffer must remain valid (ie do not free) whilst compression is in progress (state == eZDefState_Deflating)
		virtual void										SetOutputBuffer(char *pInBuffer, int inSize)=0;

		// Description:
		//	 Returns the number of bytes from the output buffer that are ready to be consumed. After consuming any output, you should call SetOutputBuffer() again to mark the buffer as available
		virtual int											GetBytesOutput()=0;

		// Description:
		//	 Begins compressing the source data pInSource of length inSourceSize to a previously specified output buffer
		//	 Only valid to be called if the stream is in state eZDefState_AwaitingInput
		//	 The specified buffer must remain valid (ie do not free) whilst compression is in progress (state == eZDefState_Deflating)
		virtual void										Input(const char *pInSource, int inSourceSize)=0;

		// Description:
		//	 Finishes the compression, causing all data to be flushed to the output buffer
		//	 Once called no more data can be input
		//	 After calling the caller must wait until GetState() reutrns eZDefState_Finished
		virtual void										EndInput()=0;

		// Description:
		//	 Returns the state of the stream,
		virtual EZDeflateState					GetState()=0;

		// Description:
		//	 Gets stats on deflate stream, valid to call at anytime
		virtual void										GetStats(SStats *pOutStats)=0;

		// Description:
		//	 Deletes the deflate stream. Will assert if stream is in an invalid state to be released (in state eZDefState_Deflating)
		virtual void										Release()=0;
		// </interfuscator:shuffle>
};

// md5 support structure
struct SMD5Context
{
	uint32					buf[4];
	uint32					bits[2];
	unsigned char		in[64];
};

struct IZLibCompressor
{
	protected:
		virtual													~IZLibCompressor()	{};		// use Release()

	public:
		// <interfuscator:shuffle>
		// Description:
		//	 Creates a deflate stream to compress data using zlib
		//	 See documentation for zlib deflateInit2() for usage details
		//	 inFlushMethod is passed to calls to zlib deflate(), see zlib docs on deflate() for more details
		virtual IZLibDeflateStream			*CreateDeflateStream(int inLevel, EZLibMethod inMethod, int inWindowBits, int inMemLevel, EZLibStrategy inStrategy, EZLibFlush inFlushMethod)=0;

		virtual void										Release()=0;

		// Description:
		//   Initializes an MD5 context
		virtual void										MD5Init( SMD5Context *pIOCtx )=0;

		// Description:
		//   Digests some data into an existing MD5 context
		virtual void										MD5Update( SMD5Context *pIOCtx, const char *pInBuff, unsigned int len )=0;

		// Description:
		//   Closes the MD5 context and extract the final 16 byte MD5 digest value
		virtual void										MD5Final( SMD5Context *pIOCtx, char outDigest[16] )=0;
		// </interfuscator:shuffle>
};

#endif // __IZLIBCOMPRESSOR_H__

