// Konstantinos Theofilatos (2019)
// based on example code from Emilio Meschi
// compile with: g++ -std=c++11 runMonitor.cc -o RunMonitor.exe
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <string.h>

#include <vector>
#include <fstream>
#include <iostream>


bool debug(false);

uint32_t bxCounts[3566]; // monitor 1-3564 bx, bx[0] is underflow, bx[3565] is overflow

struct muon{
  uint32_t f;
  uint32_t s;
};

struct block{
  uint32_t bx;
  uint32_t orbit;
  muon mu[16];
};

struct masks{
  static constexpr  uint32_t phiext  = 0x3ff;
  static constexpr  uint32_t pt      = 0x1ff;
  static constexpr  uint32_t qual    = 0xf;
  static constexpr  uint32_t etaext  = 0x1ff;
  static constexpr  uint32_t etaextv = 0xff;
  static constexpr  uint32_t etaexts = 0x100;
  static constexpr  uint32_t   iso   = 0x3;
  static constexpr  uint32_t   chrg  = 0x1;
  static constexpr  uint32_t   chrgv = 0x1;
  static constexpr  uint32_t   index = 0x7f;
  static constexpr  uint32_t   phi   = 0x3ff;
  static constexpr  uint32_t   eta   = 0x1ff;
  static constexpr  uint32_t   etav  = 0xff;
  static constexpr  uint32_t   etas  = 0x100;
  static constexpr  uint32_t   phiv  = 0x1ff;
  static constexpr  uint32_t   phis  = 0x200;
  static constexpr  uint32_t   rsv   = 0x3;
};

struct shifts{
  static constexpr  uint32_t  phiext = 0;
  static constexpr  uint32_t  pt     = 10;
  static constexpr  uint32_t  qual   = 19;
  static constexpr  uint32_t  etaext = 23;
  static constexpr  uint32_t  iso    = 0;
  static constexpr  uint32_t  chrg   = 2;
  static constexpr  uint32_t  chrgv  = 3;
  static constexpr  uint32_t  index  = 4;
  static constexpr  uint32_t  phi    = 11;
  static constexpr  uint32_t  eta    = 21;
  static constexpr  uint32_t  rsv    = 30;
};

struct gmt_scales{
  static constexpr float pt_scale  = 0.5;
  static constexpr float phi_scale = 2.*M_PI/576.;
  static constexpr float eta_scale = 0.0870/8; //9th MS bit is sign
  static constexpr float phi_range = M_PI;
};

struct header_shifts{
  static constexpr uint32_t bxmatch    = 24;
  static constexpr uint32_t mAcount    = 16;
  static constexpr uint32_t orbitmatch = 8;
  static constexpr uint32_t mBcount    = 0;
};



struct header_masks{
  static constexpr uint32_t bxmatch    = 0xff << header_shifts::bxmatch;
  static constexpr uint32_t mAcount    = 0xf  << header_shifts::mAcount;
  static constexpr uint32_t orbitmatch = 0xff << header_shifts::orbitmatch;
  static constexpr uint32_t mBcount    = 0xf;
};

bool goodBX(uint32_t bx)
{
  bool res = false;
  if(bx >= 3445 &&  bx <= 3555) res = true;
  return res;
}

int main( int argc, char* argv[] ){

  std::cout << "argc = " << argc <<  std::endl;
  for (int i = 0; i < argc; ++i) std::cout << "argv[" << i << "] = " << argv[i] << "\n";   
  std::cout << std::endl;
  
  if (argc < 3) { 
        std::cerr << "Usage: " << argv[0] << " inFile  destdir [maxLines = -1]" << std::endl;
        std::cerr << "Compile with: g++ -std=c++11 runMonitor.cc -o RunMonitor.exe " << std::endl;
        std::cerr << "./RunMonitor.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./" << std::endl;
        std::cerr << "./RunMonitor.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ 15    -> will just analyze 15 lines" << std::endl;
        return 1;
  }
  // argv[0] is the name of the executable
  // argv[1] is the filename
  // argv[2] is the destdir
  // argv[3] is the number of lines to be processed
  
  // set filename -- argv[1]
  char filename[strlen(argv[1])+1];
  strcpy(filename,argv[1]);
 
  // set destdir -- argv[2]
  char destdir[strlen(argv[2])+1];
  strcpy(destdir,argv[2]);
  
  // setm axLines -- argv[3]
  int nOutLines   = 0;
  int maxOutLines = -1;
  if(argc >= 4) maxOutLines = atoi(argv[3]);

  char *basename = strrchr(argv[1],'/');
  char *stem = strtok(basename,".");
  char *stemnonumeral = strtok(stem,"_");
  char *runnumber = strtok(0,"_");
  
  std::cout << "filename = " <<  filename << std::endl;
  std::cout << "destdir = " <<  destdir << std::endl;
  std::cout << "basename = " <<  basename << std::endl;
  std::cout << "stem = " <<  stem << std::endl;
  std::cout << "stemnonumeral = " <<  stemnonumeral << std::endl;
  std::cout << "runnumber = " <<  runnumber << std::endl;


  // -- initialize counters
  uint32_t orbit=0;
  uint32_t bx=0;
  uint64_t discarded = 0;
  for(int i = 0; i<3566 ; ++i)bxCounts[i] = 0;

  // -- get the input file
  fprintf(stderr,"opening file %s\n",filename);
  FILE *f=fopen(filename,"r");
  block bl;

  // -- loop
  uint32_t header;
  int ibytes = 0;
  while(ibytes=fread(&header,1,sizeof(uint32_t),f))
  {
    if(maxOutLines>0) // limit output to maxOutLines, default is maxOutLines = -1
    {
       nOutLines ++;
       if(nOutLines > maxOutLines) break;
    }
    uint32_t mAcount = (header & header_masks::mAcount)>>header_shifts::mAcount;
    uint32_t mBcount = (header & header_masks::mBcount)>>header_shifts::mBcount;
    fread(&bl,1,8+(mAcount+mBcount)*8,f);
    bx = bl.bx;
    if (bl.bx>0xf0000000) bx= bl.bx-0xf0000000; 
    orbit = bl.orbit;

    if (bx < 1) bxCounts[0]++;
    if (bx > 3564) bxCounts[3565]++;
    if (bx >= 1 && bx <= 3564) bxCounts[bx]++;
 
  }// end of while loop

  fclose(f);
  std::cout << "number of empty (bx, orbits) = " << discarded << std::endl;

  // create CSV file
  char txtfilename[strlen(destdir)+strlen(filename)+1];
  sprintf(txtfilename,"%s/scout_%s_%06d.monitor.txt",destdir,  runnumber, 0);
  std::ofstream   out(txtfilename, std::ios_base::trunc); // don't append, any contents that existed in the file before it is open are discarded.
  fprintf(stderr,"created txt file %s\n", txtfilename);

  // writing in the outputfile
  for(int i = 0; i<3566 ; ++i) out << bxCounts[i] << std::endl;

  // closing the file
  out.close();
 
  // -- endOf CSV's header
    
  perror("blah");
}
