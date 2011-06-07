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
#include <vector>
#include <string>
class DataRecorder
{
  public:
    DataRecorder();

    int addChannel_v3(const std::string name);

    int record_v3(int channel_index, double x, double y, double z);

  private:
    typedef double v1;
    typedef double v3[3];
    typedef double v3x3[3];

    unsigned current_timestamp;

    enum {
      ct_v1,
      ct_v3,
      ct_v3x3
    };

    struct hdr_channel {
      std::string name;

      int type;
      int data_index;

      int  n_entries;
      int  allocated;

      int  next_entry;
    };

    struct channel_v1 {
      unsigned    timestamp;
      v1 *        entries;
    };

    struct channel_v3 {
      unsigned    timestamp;
      v3 *        entries;
    };

    struct channel_v3x3 {
      unsigned    timestamp;
      v3 *        entries;
    };


    std::vector<hdr_channel*> channels;

    std::vector<channel_v3*>  v3_data;

};

