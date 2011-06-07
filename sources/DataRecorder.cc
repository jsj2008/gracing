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
#include <assert.h>

#include "gmlog.h"
#include "DataRecorder.h"

enum {
  chunk_size=100*1024
};
DataRecorder::DataRecorder()
{
  current_timestamp=0;
}

int DataRecorder::record_v3(int channel_index, double x, double y, double z)
{
#ifdef CHECK_INDICES
  if(channel_index >= channels.size()) 
    return;
#endif
  hdr_channel * hdr=channels[channel_index];

  if(hdr->type != ct_v3) 
    return -1;

  if(hdr->next_entry == hdr->n_entries) {
    assert(0);
  }

  //channel_v3 * ch=v3_data[hdr->data_index];
}

int DataRecorder::addChannel_v3(const std::string name)
{
  int data_index=v3_data.size();
  int index=channels.size();
  channel_v3 * ch=new channel_v3;
  hdr_channel * hch=new hdr_channel;

  ch->entries=new v3[chunk_size];
  hch->name=name;
  hch->type=ct_v3;
  hch->n_entries=chunk_size;
  hch->data_index=data_index;
  hch->next_entry=0;

  channels.push_back(hch);
  v3_data.push_back(ch);

  return index;
}
