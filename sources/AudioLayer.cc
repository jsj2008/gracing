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
#define BUFFER_SIZE (8*4096)


static AudioLayer * instance=0;

static ALuint       g_buffers[2];
static ALCdevice  * g_device;
static ALCcontext * g_context;
static ALuint       g_source;
static ALenum       g_format;     // internal g_format
static OggVorbis_File  g_oggStream;     // stream handle
static vorbis_info*    g_vorbisInfo;    // some formatting data
static vorbis_comment* g_vorbisComment; // user comments

static unsigned  g_bufferDuration=50;
static bool      g_songLoaded=false;
static bool      g_playing=false;


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
  GM_LOG("super\n");
  if(!instance)
    instance = new AudioLayer();
  GM_LOG("saper\n");
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

void AudioLayer::loadSong(const char * songFileName)
{
  m_commands.put(Command(cmd_loadSong,songFileName,0));
}

bool AudioLayer::executeCommand(Command & cmd)
{
  switch(cmd.m_cmd) {
    case cmd_loadSong:
      _loadSong(cmd.m_arg0);
      break;
    case cmd_startSong:
      _startSong();
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

  alSourcef(g_source, AL_GAIN, 1.0);

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
      cmd.dump();
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
