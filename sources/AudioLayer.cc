//  gracing - an idiot (but physically powered) racing game 
//  Copyright (C) 2010 gianni masullo
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// https://computing.llnl.gov/tutorials/pthreads/exercise.html
#include "AudioLayer.h"

#include "gmlog.h"
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
//#include <OpenAL/alut.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>

#include "ResourceManager.h"
#define BUFFER_SIZE   (8*4096)
#define MAX_N_BUFFERS 16

struct SoundBuffer {
  ALuint          buffer;
  ALuint          source;
  bool            used;
  float           gain;

  SoundBuffer()
  {
    buffer=0;
    used=false;
  };
};


static AudioLayer *    instance=0;
static ALuint          g_buffers[2];
static ALCdevice  *    g_device;
static ALCcontext *    g_context;
static ALuint          g_source;
static ALenum          g_format;     // internal g_format
static OggVorbis_File  g_oggStream;     // stream handle
static vorbis_info*    g_vorbisInfo;    // some formatting data
static vorbis_comment* g_vorbisComment; // user comments
static unsigned        g_bufferDuration=50;
static bool            g_songLoaded=false;
static bool            g_playing=false;
enum                   { MAX_N_SOUNDS=16 };
static SoundBuffer     g_samples[MAX_N_SOUNDS];



static const char * errorString(int code)
{
  switch(code)
  {
    case OV_EREAD:
      return ("Read from media.");
    case OV_ENOTVORBIS:
      return ("Not Vorbis data.");
    case OV_EVERSION:
      return ("Vorbis version mismatch.");
    case OV_EBADHEADER:
      return ("Invalid Vorbis header.");
    case OV_EFAULT:
      return ("Internal logic fault (bug or heap/stack corruption.");
    default:
      return ("Unknown Ogg error.");
  }
  return "";
}

static bool check()
{
  int error = alGetError();

  if(error != AL_NO_ERROR) {
    GM_LOG("error: %d (%s)\n",error,errorString(error));
    return false;
  }
  return true;
}


AudioLayer * AudioLayer::getInstance()
{
  if(!instance)
    instance = new AudioLayer();
  return instance;
}

AudioLayer::AudioLayer()
{
  GM_LOG("creating audiolayer\n");

  g_bufferDuration=1000;
  g_songLoaded=false;
  g_playing=false;
  g_device = alcOpenDevice(NULL); // select the "preferred g_device"

  if(!g_device) {
    check();
    return;
  }

  g_context=alcCreateContext(g_device,NULL);

  if(!g_context) {
    check();
    return;
  }

  alcMakeContextCurrent( g_context );
  check();
}

void AudioLayer::setFxVolume(float f)
{
  m_commands.put(Command(cmd_setFxVolume,f));
}

void AudioLayer::setSongVolume(float f)
{
  m_commands.put(Command(cmd_setSongVolume,f));
}

void AudioLayer::loadSong(const char * songFileName)
{
  m_commands.put(Command(cmd_loadSong,songFileName,0));
}

void AudioLayer::enableFx(bool  enable)
{
  m_commands.put(Command(cmd_enableSamples,0,enable));
}

void AudioLayer::loadSample(unsigned index, const char * sampleFileName) 
{
  m_commands.put(Command(cmd_loadSample,sampleFileName,index));
}

bool AudioLayer::executeCommand(Command & cmd)
{
  switch(cmd.m_cmd) {
    case cmd_loadSong:
      _loadSong(cmd.m_arg0);
      break;
    case cmd_loadSample:
      _loadSample(cmd.m_arg1,cmd.m_arg0);
      break;
    case cmd_playSample:
      _playSample(cmd.m_arg1);
      break;
    case cmd_startSong:
      _startSong();
      break;
    case cmd_setSongVolume:
      _setSongVolume(cmd.m_arg2);
      break;
    case cmd_setFxVolume:
      _setFxVolume(cmd.m_arg2);
      break;
    case cmd_enableSamples:
      _enableSamples(cmd.m_arg1);
      break;
  }
  return false;
}


void AudioLayer::_startSong()
{
  if(!g_songLoaded)
    return;

  g_playing=true;
}

void AudioLayer::_setFxVolume(float volume)
{
  for(unsigned i=0; i<MAX_N_BUFFERS; i++) 
    if(g_samples[i].used) {
      GM_LOG("setting fx volume %f\n",volume);
      g_samples[i].gain = volume;
      alSourcef(g_samples[i].source, AL_GAIN, g_samples[i].gain);
    }
}

void AudioLayer::_enableSamples(unsigned v)
{
  if(v) {
    for(unsigned i=0; i<MAX_N_BUFFERS; i++) 
      if(g_samples[i].used) 
        alSourcef(g_samples[i].source, AL_GAIN, g_samples[i].gain);
  } else {
    for(unsigned i=0; i<MAX_N_BUFFERS; i++) 
      if(g_samples[i].used) 
        alSourcef(g_samples[i].source, AL_GAIN, 0);
  }
}

void AudioLayer::_setSongVolume(float f)
{
  alSourcef(g_source, AL_GAIN, f);
}

void AudioLayer::_loadSong(const char * songFileName)
{
  FILE*  oggFile;       // file handle

  if(g_songLoaded) {
    GM_LOG("deleting previous song\n");
    // TODO: release song resources
  }


  GM_LOG("loading song '%s'\n",songFileName);

  int result;

  if(!(oggFile = fopen(songFileName, "rb"))) {
    GM_LOG("Could not open Ogg file '%s'\n",songFileName);
    return;
  }

  if((result = ov_open(oggFile, &g_oggStream, NULL, 0)) < 0)
  {
    fclose(oggFile);

    GM_LOG("Could not open Ogg stream. %d ",result); // + errorString(result);
    return ;
  }

  g_vorbisInfo = ov_info(&g_oggStream, -1);
  g_vorbisComment = ov_comment(&g_oggStream, -1);

  if(g_vorbisInfo->channels == 1)
    g_format = AL_FORMAT_MONO16;
  else
    g_format = AL_FORMAT_STEREO16;

  alGenBuffers(2, g_buffers);
  if(!check()) {
    GM_LOG("onco\n");
    return;
  }

  alGenSources(1, &g_source);
  if(!check()) {
    GM_LOG("anco\n");
    return;
  }

  double gain=1.;

  ResourceManager::getInstance()->cfgGet("audio/music-volume",gain);
  alSourcef(g_source, AL_GAIN, gain);

  alSource3f(g_source, AL_POSITION,        0.0, 0.0, 0.0);
  alSource3f(g_source, AL_VELOCITY,        0.0, 0.0, 0.0);
  alSource3f(g_source, AL_DIRECTION,       0.0, 0.0, 0.0);
  alSourcef (g_source, AL_ROLLOFF_FACTOR,  0.0          );
  alSourcei (g_source, AL_SOURCE_RELATIVE, AL_TRUE      );


  unsigned samplePerSec = g_vorbisInfo->rate * g_vorbisInfo->channels;
  g_bufferDuration=(1000 * BUFFER_SIZE) / samplePerSec;
  g_bufferDuration=100;

  g_songLoaded=true;
}

void AudioLayer::displaySongInfo()
{
  GM_LOG( "version         %d\n", g_vorbisInfo->version);
  GM_LOG( "channels        %d\n", g_vorbisInfo->channels);
  GM_LOG( "rate (hz)       %ld\n", g_vorbisInfo->rate);
  GM_LOG( "bitrate upper   %ld\n", g_vorbisInfo->bitrate_upper);
  GM_LOG( "bitrate nominal %ld\n", g_vorbisInfo->bitrate_nominal);
  GM_LOG( "bitrate lower   %ld\n", g_vorbisInfo->bitrate_lower);
  GM_LOG( "bitrate window  %ld\n", g_vorbisInfo->bitrate_window);
  GM_LOG( "vendor '%s'\n",g_vorbisComment->vendor);

  for(int i = 0; i < g_vorbisComment->comments; i++)
    GM_LOG("  %s \n", g_vorbisComment->user_comments[i]);
}


void AudioLayer::playback()
{
  if(isPlaying()) {
    return;
  }

  if(!stream(g_buffers[0])) {
    return;
  }

  if(!stream(g_buffers[1])) {
    return;
  }

  alSourceQueueBuffers(g_source, 2, g_buffers);
  alSourcePlay(g_source);
}

void AudioLayer::startSong()
{

  m_commands.put(Command(cmd_startSong,0,0));

#if 0
  if(!g_songLoaded)
    return;
  g_playing=true;
#endif
}

bool AudioLayer::stream(ALuint buffer) 
{
  char data[BUFFER_SIZE];
  int  size = 0;
  int  section;
  int  result;

  while(size < BUFFER_SIZE)
  {
    result = ov_read(&g_oggStream, data + size, BUFFER_SIZE - size, 0, 2, 1, & section);

    if(result > 0)
      size += result;
    else
      if(result < 0) {
        GM_LOG("cannot read file\n");
        break;
      } else {
        GM_LOG("zgna\n");
        break;
      }
  }

  if(size == 0)
    return false;

  alBufferData(buffer, g_format, data, size, g_vorbisInfo->rate);

  return check();
}

bool AudioLayer::update()
{
  int processed;
  bool active = true;

  alGetSourcei(g_source, AL_BUFFERS_PROCESSED, &processed);

  while(processed--)
  {
    ALuint buffer;

    alSourceUnqueueBuffers(g_source, 1, &buffer);

    active = stream(buffer);

    alSourceQueueBuffers(g_source, 1, &buffer);
  }

  return active;
}


void AudioLayer::run() 
{
  irr::IrrlichtDevice *device =
    ResourceManager::getInstance()->getDevice();



  bool done=false;
  g_songLoaded = false;
  while(!done) {
    if(!m_commands.isEmpty()) {
      Command cmd;
      m_commands.get(cmd);
      //cmd.dump();
      done=executeCommand(cmd);
    }

    if(g_playing) {
      if(!update()) {
        GM_LOG("song terminated\n");
        g_playing=false;
      } else {
        if(!isPlaying())
          playback();
      }
    }
    device->sleep(50);
  }
}

void AudioLayer::playSample(unsigned index)
{
  m_commands.put(Command(cmd_playSample,0,index));
}



bool AudioLayer::isPlaying()
{
  ALenum state;

  alGetSourcei(g_source, AL_SOURCE_STATE, &state);

  return (state == AL_PLAYING);
}

AudioLayer::~AudioLayer()
{
}

void AudioLayer::stopSong()
{
  GM_LOG("stop playing song\n");
}

void AudioLayer::_freeSample(unsigned index)
{
  if(index >= MAX_N_SOUNDS)
    return;
  if(!g_samples[index].used)
    return;

  // ... free the openAl sample
}

void AudioLayer::_playSample(unsigned index)
{
  ALenum state;
  if(index >= MAX_N_SOUNDS)
    return;

  alGetSourcei(g_samples[index].source, AL_SOURCE_STATE, &state);

  if(state == AL_PLAYING)
    GM_LOG("not playing, becaouse alreadu playing\n");

  alSourcePlay(g_samples[index].source);
}

void AudioLayer::_loadSample(unsigned index, const char * name) 
{
  char xbuffer[5];
  short audioFormat;
  short channels;
  int sampleRate;
  int byteRate;
  short bitsPerSample;
  int dataSize;
  ALenum  format;
  float duration;


  if(index >= MAX_N_SOUNDS)
    return;

  _freeSample(index);

  alGenBuffers(1,  & g_samples[index].buffer);
  alGenSources(1, &g_samples[index].source);


  double gain=1.;
  bool   fxEnabled=true;

  ResourceManager::getInstance()->cfgGet("audio/fx-volume",gain);
  ResourceManager::getInstance()->cfgGet("audio/fx-enabled",fxEnabled);

  g_samples[index].gain = gain;
  g_samples[index].used = true;

    
  if(!fxEnabled)
    gain = 0.;

  alSourcef(g_source, AL_GAIN, gain);

  alSource3f(g_samples[index].source, AL_POSITION,        0.0, 0.0, 0.0);
  alSource3f(g_samples[index].source, AL_VELOCITY,        0.0, 0.0, 0.0);
  alSource3f(g_samples[index].source, AL_DIRECTION,       0.0, 0.0, 0.0);
  alSourcef (g_samples[index].source, AL_ROLLOFF_FACTOR,  0.0          );
  alSourcei (g_samples[index].source, AL_SOURCE_RELATIVE, AL_TRUE      );

  irr::io::IFileSystem *  filesystem=ResourceManager::getInstance()->getFileSystem();

  irr::io::IReadFile * rfile=filesystem->
    createAndOpenFile (name);

  if(!rfile) {
    GM_LOG("Cannot open '%s'\n",name);
    goto error_and_exit;
  }

  xbuffer[4] = '\0';
  rfile->read(xbuffer,4);
  if (strcmp(xbuffer, "RIFF") != 0) {
    GM_LOG( "Not a WAV file (missing RIFF)\n");
    goto error_close_and_exit;
  }
  Util::readInt(rfile);
  rfile->read(xbuffer,4);
  if (strcmp(xbuffer, "WAVE") != 0) {
    GM_LOG( "Not a WAV file (missing WAVE)\n");
    goto error_close_and_exit;
  }
  rfile->read(xbuffer,4);
  if (strcmp(xbuffer, "fmt ") != 0) {
    GM_LOG( "Not a WAV file (missing fmt )");
    goto error_close_and_exit;
  }
  Util::readInt(rfile);

  audioFormat = Util::readS16(rfile); 
  channels = Util::readS16(rfile); 
  sampleRate = Util::readS32(rfile);
  byteRate = Util::readS32(rfile); 
  Util::readS16(rfile);
  bitsPerSample = Util::readS16(rfile);


  if(bitsPerSample != 16) {
    
  }

  rfile->read(xbuffer,4);
  if(strcmp(xbuffer,"data") != 0) {
    GM_LOG("Invalid WAF file (missing 'data')\n");
    goto error_close_and_exit;
  }

  dataSize=Util::readS16(rfile);

  GM_LOG("Audio format : %d\n",audioFormat);
  GM_LOG("Channers     : %d\n",channels);
  GM_LOG("Sample rate  : %d\n",sampleRate);
  GM_LOG("Byte rate    : %d\n",byteRate);
  GM_LOG("Bits/Sample  : %d\n",bitsPerSample);
  GM_LOG("Data size    : %d\n",dataSize);

  g_samples[index].used=true;

  unsigned char * data;
  data=new unsigned char [dataSize];

  rfile->read(data,dataSize);

  duration = float(dataSize) / byteRate;

  format = AL_FORMAT_MONO16;

  alBufferData(g_samples[index].buffer, format, data, dataSize, sampleRate);
  free(data);
  alSourceQueueBuffers(g_samples[index].source, 1, &g_samples[index].buffer);


error_close_and_exit:
  rfile->drop();
error_and_exit:
  return;

#if 0
  if (audioFormat != 16) {
    short extraParams = file_read_int16_le(xbuffer, file);
    file_ignore_bytes(file, extraParams);
  }

  if (fread(xbuffer, sizeof(char), 4, file) != 4 || strcmp(xbuffer, "data") != 0)
    throw "Invalid WAV file";

  int dataChunkSize = file_read_int32_le(xbuffer, file);
  unsigned char* bufferData = file_allocate_and_read_bytes(file, (size_t) dataChunkSize);

  float duration = float(dataChunkSize) / byteRate;
  alBufferData(buffer, GetFormatFromInfo(channels, bitsPerSample), bufferData, dataChunkSize, sampleRate);
  free(bufferData);
  fclose(f);
#endif
}

