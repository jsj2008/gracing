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
#ifndef THREAD_H
#define THREAD_H

#include <vector>

#ifdef THREAD_USE_POSIX
#include <pthread.h>
#endif

class Thread {
  public:
    Thread();
    virtual ~Thread();
    virtual void run()=0;

#ifdef THREAD_USE_POSIX

  private:
    // i know this should not be here
    // to preserve indipendence from
    // the underlying platform used.
    pthread_t  m_threadId;
#else
  #error Threads not supported on this platform
#endif
};

#ifdef THREAD_USE_POSIX
template<class T>
class SimpleSyncQueue
{
  public:
    SimpleSyncQueue()
    {
      pthread_mutex_init(&m_mutex,0);
    }
    void put(const T & msg)
    {
      pthread_mutex_lock(&m_mutex);
      m_vct.push_back(msg);
      pthread_mutex_unlock(&m_mutex);
    }

    bool get(T & msg)
    {
      bool ret=false;
      pthread_mutex_lock(&m_mutex);
      if(m_vct.size()) {
        msg = m_vct[0];
        m_vct.erase(m_vct.begin());
        ret=true;
      }
      pthread_mutex_unlock(&m_mutex);
      return ret;
    }
    bool isEmpty() 
    {
      bool ret;
      pthread_mutex_lock(&m_mutex);
      ret = m_vct.size() == 0;
      pthread_mutex_unlock(&m_mutex);
      return ret;
    }

  private:
    std::vector<T> m_vct;

    pthread_mutex_t m_mutex;

};
#endif

#endif
