#include "mp4player.h"
#include "log.h"
#include "codecs.h"
#include "g711/g711codec.h"
#include "h263/h263codec.h"
#include "h264/h264decoder.h"

MP4Player::MP4Player() : streamer(this)
{
	audioCodec = NULL;
	videoDecoder = NULL;
}

MP4Player::~MP4Player()
{
	//Delete codecs
	if (audioCodec)
		delete (audioCodec);
	if (videoDecoder)
		delete (videoDecoder);
}

int MP4Player::Init(AudioOutput *audioOutput,VideoOutput *videoOutput,TextOutput *textOutput)
{
	//Sava
	this->audioOutput = audioOutput;
	this->videoOutput = videoOutput;
	this->textOutput = textOutput;
}

int MP4Player::Play(const char* filename,bool loop)
{
	Log("-MP4Player play [\"%s\"]\n",filename);

	//Stop just in case
	streamer.Close();

	//Open file
	if (!streamer.Open(filename))
		//Error
		return Error("Error opening mp4 file");

	//Save loop value
	this->loop = loop;

	//Open audio codec
	if (streamer.HasAudioTrack())
		//Depending on the codec
		switch (streamer.GetAudioCodec())
		{
			case AudioCodec::PCMA:
				//Create audio codec
				audioCodec = new PCMACodec();
				break;
			case AudioCodec::PCMU:
				//Create audio codec
				audioCodec = new PCMACodec();
				break;
		}
	//Open video codec
	if (streamer.HasVideoTrack())
		//Depending on the codec
		switch (streamer.GetVideoCodec())
		{
			case VideoCodec::H263_1998:
				//Create audio codec
				videoDecoder = new H263Decoder();
				break;
			case VideoCodec::H264:
				//Create audio codec
				videoDecoder = new H264Decoder();
				break;
		}
		
	//Start playback
	return streamer.Play();
}

int MP4Player::Stop()
{
	Log("-MP4Player stop\n");

	//Do not loop anymore
	loop = false;

	//Stop
	streamer.Stop();
	
	//Close
	streamer.Close();

	return 1;
}

int MP4Player::End()
{
}

void MP4Player::onTextFrame(TextFrame &text)
{
	Log("-On TextFrame [\"%ls\"]\n",text.GetWChar());

	//Check textOutput
	if (textOutput)
		//Publish it
		textOutput->SendFrame(text);
}

void MP4Player::onRTPPacket(RTPPacket &packet)
{
	WORD buffer[1024];
	DWORD bufferSize = 1024;
	//Get data
	BYTE *data = packet.GetMediaData();
	//Get leght
	DWORD len = packet.GetMediaLength();
	//Get mark
	bool mark = packet.GetRTPHeader()->m;
	
	//Depending on the media
	switch (packet.GetMedia())
	{
		case MediaFrame::Audio:
			//Check decoder
			if (!audioCodec || !audioOutput)
				//Do nothing
				return;

			//Decode it
			len = audioCodec->Decode(data,len,buffer,bufferSize);

			//Play it
			audioOutput->PlayBuffer((WORD *)buffer,len,0);

			break;
		case MediaFrame::Video:
			//Check decoder
			if (!videoDecoder || !videoOutput)
				//Do nothing
				return;
			
			//Decode packet
			if(!videoDecoder->DecodePacket(data,len,false,mark))
				//Error
				return;
			//Check if it is last
			if(mark)
			{
				//Get dimensions
				DWORD width = videoDecoder->GetWidth();
				DWORD height= videoDecoder->GetHeight();
				//Set it
				videoOutput->SetVideoSize(width,height);
				//Set decoded frame
				videoOutput->NextFrame(videoDecoder->GetFrame());
			}
			break;
	}

}

void MP4Player::onMediaFrame(MediaFrame &frame)
{
	//Do nothing now
}

void MP4Player::onEnd()
{
	//If in loop
	if (loop)
		//Play again
		streamer.Play();
}
