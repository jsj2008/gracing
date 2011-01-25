#ifndef UTIL_HH
#define UTIL_HH

inline double rad2deg(double value)
{
  return 57.2957795 * value;
}

class Util 
{
  public:
  static inline int readInt(irr::io::IReadFile * file) 
  {
    int val;
    file->read(&val,sizeof(val));
    return val;
  }

  // NB: the string must be deleted!!!
  static inline char * readString(irr::io::IReadFile * file)
  {
    unsigned len=readShort(file);
    char * str=new char[len+1];
    file->read(str,len);
    str[len]=0;
    return str;
  }

  static inline unsigned readShort(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline void readTriple(irr::io::IReadFile * file, double * tr) 
  {
    tr[0]=readDouble(file);
    tr[1]=readDouble(file);
    tr[2]=readDouble(file);
  }

  static inline unsigned readMark(irr::io::IReadFile * file)
  {
    unsigned short val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline double readDouble(irr::io::IReadFile * file)
  {
    double val;
    file->read(&val,sizeof(val));
    return val;
  }

  static inline void readVertex(irr::io::IReadFile * file, irr::core::vector3df & vec)
  {
    vec.X=readDouble(file);
    vec.Y=readDouble(file);
    vec.Z=readDouble(file);
  }

};

#endif
