// Konstantinos Theofilatos (2019)
// based on example code from Emilio Meschi
// compile with: g++ -std=c++11 readBinary.cc -o run.out
// run with: ./run.out /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat 0 ./
// one line: g++ -std=c++11 readBinary.cc -o run.out; ./run.out /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat 0 ./
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <string.h>

#include <vector>
#include <fstream>
#include <iostream>


bool debug(false);

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
  static constexpr  uint32_t phiext = 0x3ff;
  static constexpr  uint32_t pt = 0x1ff;
  static constexpr  uint32_t qual = 0xf;
  static constexpr  uint32_t etaext = 0x1ff;
  static constexpr  uint32_t etaextv = 0xff;
  static constexpr  uint32_t etaexts = 0x100;
  static constexpr  uint32_t   iso = 0x3;
  static constexpr  uint32_t   chrg = 0x1;
  static constexpr  uint32_t   chrgv = 0x1;
  static constexpr  uint32_t   index = 0x7f;
  static constexpr  uint32_t   phi = 0x3ff;
  static constexpr  uint32_t   eta = 0x1ff;
  static constexpr  uint32_t   etav = 0xff;
  static constexpr  uint32_t   etas = 0x100;
  static constexpr  uint32_t   phiv = 0x1ff;
  static constexpr  uint32_t   phis = 0x200;
  static constexpr  uint32_t  rsv = 0x3;
};

struct shifts{
  static constexpr  uint32_t  phiext = 0;
  static constexpr  uint32_t  pt = 10;
  static constexpr  uint32_t  qual = 19;
  static constexpr  uint32_t  etaext = 23;
  static constexpr  uint32_t  iso =	0;
  static constexpr  uint32_t  chrg = 2;
  static constexpr  uint32_t  chrgv	= 3;
  static constexpr  uint32_t  index	= 4;
  static constexpr  uint32_t  phi = 11;
  static constexpr  uint32_t  eta =	21;
  static constexpr  uint32_t  rsv =	30;
};

struct gmt_scales{
  static constexpr float pt_scale = 0.5;
  static constexpr float phi_scale = 2.*M_PI/576.;
  static constexpr float eta_scale = 0.0870/8; //9th MS bit is sign
  static constexpr float phi_range = M_PI;
};

struct header_shifts{
  static constexpr uint32_t bxmatch=24;
  static constexpr uint32_t mAcount=16;
  static constexpr uint32_t orbitmatch=8;
  static constexpr uint32_t mBcount=0;
};



struct header_masks{
  static constexpr uint32_t bxmatch = 0xff<<header_shifts::bxmatch;
  static constexpr uint32_t mAcount = 0xf<<header_shifts::mAcount;
  static constexpr uint32_t orbitmatch = 0xff<<header_shifts::orbitmatch;
  static constexpr uint32_t mBcount = 0xf;
};


int main( int argc, char* argv[] ){

 // for (int i = 0; i < argc; ++i) std::cout << argv[i] << "\n";   
 // std::cout << std::endl;
  /*
  if (argc < 3) { // We expect 3 arguments: the program name, the source path and the destination path
        std::cerr << "Usage: " << argv[0] << "SOURCE DESTINATION" << std::endl;
        return 1;
  }
  */

  char filename[strlen(argv[1])+1];
  char destdir[strlen(argv[3])+1];
  int offset=atoi(argv[2]);
  strcpy(filename,argv[1]);
  strcpy(destdir,argv[3]);

  char *basename = strrchr(argv[1],'/');
  char *stem = strtok(basename,".");
  char *stemnonumeral = strtok(stem,"_");
  char *runnumber = strtok(0,"_");
  char *ordinal_c = strtok(0,"_");
  int ordinal = atoi(ordinal_c)+offset;

  
  std::cout << "filename = " <<  filename << std::endl;
  std::cout << "destdir = " <<  destdir << std::endl;
  std::cout << "offset = " <<  offset << std::endl;
  std::cout << "basename = " <<  basename << std::endl;
  std::cout << "stem = " <<  stem << std::endl;
  std::cout << "stemnonumeral = " <<  stemnonumeral << std::endl;
  std::cout << "runnumber = " <<  runnumber << std::endl;
  std::cout << "ordinal_c = " <<  ordinal_c << std::endl;
  std::cout << "ordinal = " <<  ordinal << std::endl;

  char csvfilename[strlen(destdir)+strlen(filename)+1];
  sprintf(csvfilename,"%s/scout_%s_%06d.csv",destdir,runnumber,ordinal);
  std::ofstream   out(csvfilename, std::ios_base::app);

  fprintf(stderr,"created csv file %s\n",csvfilename);
  // writing the header of the CSV file
  out << "orbit" << "," << "bx"  << "," << "nMu" << "," << "phi" << "," << "phip" << "," << "eta" << "," << "etap" << "," << "pt" << "," << "qual"
	      << "," << "charge" << std::endl;


  std::vector<float> vpt;
  std::vector<float> veta;
  std::vector<float> vphi;
  std::vector<float> vetap;
  std::vector<float> vphip;
  std::vector<int> vcharge;
  std::vector<int> vindex;
  std::vector<int> vqual;

  std::vector<int> vieta;
  std::vector<int> vietaext;

  uint32_t orbit=0;
  uint32_t bx=0;
  uint32_t block_count = 0;
  uint64_t discarded = 0;
  uint64_t full = 0;
  fprintf(stderr,"opening file %s\n",filename);
  
  FILE *f=fopen(filename,"r");
  block bl;

  int eventcount = 0;
  int ibytes = 0;
  uint32_t header;
  float pt_cut= 1.5;
  uint32_t qual_cut = 12;
  while(ibytes=fread(&header,1,sizeof(uint32_t),f)){
    //    fprintf(stderr, "0x%x read %d\n",header,ibytes);
    uint32_t mAcount = (header & header_masks::mAcount)>>header_shifts::mAcount;
    uint32_t mBcount = (header & header_masks::mBcount)>>header_shifts::mBcount;
    fread(&bl,1,8+(mAcount+mBcount)*8,f);
    vpt.clear();
    veta.clear();
    vphi.clear();
    vetap.clear();
    vphip.clear();
    vcharge.clear();
    vindex.clear();
    vqual.clear();
    vieta.clear();
    vietaext.clear();
    bx = bl.bx;
    orbit = bl.orbit;
    if(mAcount+mBcount==16) full+=1;
    for(unsigned int i=0;i<mAcount+mBcount; i++){
      uint32_t ipt = (bl.mu[i].f >> shifts::pt) & masks::pt;
      uint32_t qual = (bl.mu[i].f >> shifts::qual) & masks::qual;
      //	if(qual<qual_cut)continue;
      float pt = (ipt-1)*gmt_scales::pt_scale;
      //	if(pt<pt_cut) continue;
      
      int32_t iphiext = ((bl.mu[i].f >> shifts::phiext) & masks::phiext);
      float phiext=iphiext*gmt_scales::phi_scale;
      if(phiext>M_PI) phiext=phiext-2*M_PI;

      int32_t ietaext = ((bl.mu[i].f >> shifts::etaext) & masks::etaextv);
      if(((bl.mu[i].f >> shifts::etaext) & masks::etaexts)!=0) ietaext -= 256;
      vietaext.push_back((bl.mu[i].f >> shifts::etaext) & masks::etaext);
      float etaext = ietaext*gmt_scales::eta_scale;
      //	if(etaext == 0. &&phiext ==0.)continue;
      uint32_t iso = (bl.mu[i].s >> shifts::iso) & masks::iso;
      int32_t chrg = 0;
      if((bl.mu[i].s >> shifts::chrgv) & masks::chrgv==1)
	chrg=(bl.mu[i].s >> shifts::chrg) & masks::chrg==1 ? -1 : 1 ;
      //    uint32_t chrgv = (bl.mu[i].s >> shifts::chrgv) & masks::chrgv;
      uint32_t index = (bl.mu[i].s >> shifts::index) & masks::index;
      int32_t iphi = ((bl.mu[i].s >> shifts::phi) & masks::phi);
      float phi=iphi*gmt_scales::phi_scale;
      if(phi>M_PI) phi=phi-2*M_PI;

      int32_t ieta = (bl.mu[i].s >> shifts::eta) & masks::etav;
      if(((bl.mu[i].s >> shifts::eta) & masks::etas)!=0) ieta -= 256;
      vieta.push_back((bl.mu[i].s >> shifts::eta)& masks::eta);
      float eta = ieta*gmt_scales::eta_scale;
      vpt.push_back(pt);
      veta.push_back(eta);
      vphi.push_back(phi);
      vetap.push_back(etaext);
      vphip.push_back(phiext);
      vcharge.push_back(chrg);
      vindex.push_back(index);
      vqual.push_back(qual);
    }
    if(!vpt.empty()){

      for(unsigned int i = 0; i < vpt.size(); i++){
	  
	if(vindex[i]<36 || vindex[i]>70) continue;

	if(veta[i]<-1. && debug){
	  std::cout << " orbit, bx " << orbit << "," << bx << std::endl;
	  std::cout << "*****************************************************************" << std::endl;
	  std::cout << "index: " << vindex[i] << " :eta,ietaext " << std::hex << vieta[i] << "," << vietaext[i] << std::dec << " -- "  
		    << vphi[i] << "," << vphip[i] << "," << veta[i] << "," 
		    << vetap[i] << "," << vpt[i] << "," << vqual[i]                    
		    << "," << vcharge[i] << std::endl; 
	  std::cout << "*****************************************************************" << std::endl;
	  for(unsigned int k=0; k < vpt.size(); k++){
	      std::cout << "=================================================================" << std::endl;
	      std::cout << "index: " << vindex[k] << " :eta,ietaext " << std::hex << vieta[k] << "," << vietaext[k] << std::dec << " -- "  
			<< vphi[k] << "," << vphip[k] << "," << veta[k] << "," 
			<< vetap[k] << "," << vpt[k] << "," << vqual[k]                    
			<< "," << vcharge[k] << std::endl; 
	      std::cout << "=================================================================" << std::endl;

	  }
	}
	  
        // writing to the CSV
	out << orbit << "," << bx << "," << vpt.size() << "," << vphi[i] << "," << vphip[i] << "," << veta[i] << "," << vetap[i] << "," << vpt[i] << "," << vqual[i] 
	      << "," << vcharge[i] << vpt.size() << std::endl;
	  eventcount++;
      }




    }else{
      discarded++;
    }
    block_count++;
    if(block_count%10000000==0){
      for(unsigned int i = 0; i < vpt.size(); i++){
	if(vindex[i]<36 || vindex[i]>70) continue;
	std::cout << "Sample printout " << std::endl;
	std::cout << "index: " << vindex[i] << " :eta,ietaext " << std::hex << vieta[i] << "," << vietaext[i] << std::dec << "--"  
		  << vphi[i] << "," << vphip[i] << "," << veta[i] << "," 
		  << vetap[i] << "," << vpt[i] << "," << vqual[i]                    
		  << "," << vcharge[i] << std::endl;
      }
    }
    if(block_count%10000000==0){
      fprintf(stderr,"block %d\n",block_count);
      fprintf(stderr,"bxs discarded with 0 muons %d\n",discarded);
      fprintf(stderr,"bxs with full record %d\n",full);
      fprintf(stderr,"muons written %d\n",eventcount);
    }
  }
  fclose(f);


    

  perror("blah");
    
    

}
