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

#include "Thread.h"
#include "gmlog.h"

class AudioLayer : public Thread
{
   public:
     static AudioLayer * getInstance();

     virtual ~AudioLayer();

     virtual void loadSong(const char * songFileName);
     virtual void loadSample(unsigned index, const char * sampleFileName);
     virtual void playSample(unsigned index);

     virtual void enableFx(bool enabled);
     virtual void enableSong(bool enabled);

     virtual void setSongVolume(float volume);
     virtual void setFxVolume(float volume);

     virtual void startSong();
     virtual void stopSong();
     virtual void displaySongInfo();
     virtual bool isPlaying();



     virtual void run();

   private:

     void _loadSong(const char * songFileName);
     void _startSong();
     void _loadSample(unsigned index, const char * wavFileName);
     void _freeSample(unsigned index);
     void _loadSample(unsigned index);
     void _playSample(unsigned index);
     void _setSongVolume(float volume);
     void _enableSamples(unsigned value);
     void _setFxVolume(float volume);
     void _enableSong(unsigned enable);

     /* commands */
     enum {
       cmd_none,
       cmd_loadSong,
       cmd_startSong,
       cmd_stopSong,
       cmd_loadSample,
       cmd_playSample,
       cmd_setSongVolume,
       cmd_enableSong,
       cmd_enableSamples,
       cmd_setFxVolume
     };

     struct Command {
       enum { arg0_size=512 };
       unsigned char m_cmd;
       char          m_arg0[arg0_size];
       unsigned      m_arg1;
       float         m_arg2;

       Command() { m_cmd=cmd_none; }

       Command(unsigned cmd, float f)
       {
         m_cmd=cmd;
         m_arg0[0]=0;
         m_arg1=0;
         m_arg2=f;
       }

       Command(unsigned cmd, const char * arg0, unsigned arg1) 
       {
         m_cmd = cmd;
         if(arg0)
           strncpy(m_arg0,arg0,arg0_size);
         m_arg1 = arg1;
       }

       void dump() 
       {
         switch(m_cmd) {
           case cmd_none: 
             GM_LOG("none "); 
             break;
           case cmd_loadSong: 
             GM_LOG("load song '%s'\n",m_arg0); 
             break;
           case cmd_loadSample: 
             GM_LOG("load sample '%s'\n",m_arg0); 
             break;
           case cmd_startSong: 
             GM_LOG("start song\n");
             break;
           case cmd_stopSong: 
             GM_LOG("stop song\n");
             break;
           case cmd_playSample: 
             GM_LOG("play sample %d\n",m_arg1);
             break;
           case cmd_enableSamples: 
             GM_LOG("enable samples %d\n",m_arg1);
             break;
           case cmd_enableSong: 
             GM_LOG("enable samples %d\n",m_arg1);
             break;
           case cmd_setFxVolume: 
             GM_LOG("setting fx volume %f\n",m_arg2);
             break;
           default:
             GM_LOG("unknow\n");
             break;
         }
       }
     };

     SimpleSyncQueue<Command> m_commands;

     AudioLayer();
     bool executeCommand(Command & cmd);
     bool stream(unsigned );
     bool update();
     void playback();
};
