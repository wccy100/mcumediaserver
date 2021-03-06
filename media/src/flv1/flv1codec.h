#ifndef _FLV1CODEC_H_
#define _FLV1CODEC_H_
extern "C" {
#include <libavcodec/avcodec.h>
} 
#include "codecs.h"
#include "video.h"

class FLV1Encoder : public VideoEncoder
{
public:
	FLV1Encoder(int qualityMin,int qualityMax);
	virtual ~FLV1Encoder();
	virtual VideoFrame* EncodeFrame(BYTE *in,DWORD len);
	virtual int FastPictureUpdate();
	virtual int GetNextPacket(BYTE *out,DWORD &len);
	virtual int SetSize(int width,int height);
	virtual int SetFrameRate(int fps,int kbits,int intraPeriod);

private:
	int OpenCodec();

	AVCodec 	*codec;
	AVCodecContext	*ctx;
	AVFrame		*picture;
	int		bitrate;
	int		fps;
	int		format;
	int		opened;
	int		qMin;
	int		qMax;
	int		intraPeriod;
	VideoFrame* 	frame;
	DWORD		bufLen;
	DWORD		bufIni;
	DWORD		bufSize;
};

class FLV1Decoder : public VideoDecoder
{
public:
	FLV1Decoder();
	virtual ~FLV1Decoder();
	virtual int DecodePacket(BYTE *in,DWORD len,int lost,int last);
	virtual int Decode(BYTE *in,DWORD len);
	virtual int GetWidth()	{return ctx->width;};
	virtual int GetHeight()	{return ctx->height;};
	virtual int GetFPS() {return (int)round(1 / av_q2d(ctx->time_base));};
	virtual BYTE* GetFrame(){return (BYTE *)frame;};
	virtual DWORD GetFrameSize(){return frameSize;};
	virtual int GetBitRate(){return ctx->bit_rate;};
private:
	AVCodec 	*codec;
	AVCodecContext	*ctx;
	AVFrame		*picture;
	BYTE*		buffer;
	DWORD		bufLen;
	DWORD 		bufSize;
	BYTE*		frame;
	DWORD		frameSize;
	BYTE		src;
};


#endif
