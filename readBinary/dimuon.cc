// Konstantinos Theofilatos (2020)
// compile with: 
// g++ -std=c++11 dimuon.cc -o dimuon.exe `root-config --glibs --cflags`
// run it with ./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ -1 1
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <string.h>

#include <vector>
#include <fstream>
#include <iostream>

#include "TLorentzVector.h"

bool debug(false);


class L1Muon:public TLorentzVector
{
   public: 
     int   charge = 0;  
     int   index  = 0;  
     int   qual   = 0;
};


/*
template <typename T> 
class vectorVar{
  public:
  vectorVar(){n = ""; v.reset();}
  ~vectorVar(){}
  std::string n; // name
  std::vector<T> v; // vector
  void reset(){v.reset();}
};
*/
/*
class record{
    public:
    void reset();
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
};
void record::reset(){
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
    bx = 0 ;
    orbit = 0;
}
*/

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
  
  if (argc < 3) { // We expect 3 arguments: the program name, the source path and the destination path
        std::cerr << "Usage: " << argv[0] << " inFile  outFilePath [maxLines = -1] [flag = 0][offsetInFileName = 0]" << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./" << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ 15    -> will just store 15 lines" << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ 15 1  -> will just store 15 lines of barrel " << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ -1 1  -> will store all entries of the barrel " << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ -1 2  -> will just store the barrel in the abord gap" << std::endl;
        std::cerr << "./dimuon.exe /eos/cms/store/cmst3/group/daql1scout/hiion/scout_326790_000000.dat ./ 15 1 3 -> will just store 15 lines of barrel muons in scout_326790_000003.txt" << std::endl;
        return 1;
  }

  

  int nOutLines   = 0;

  char filename[strlen(argv[1])+1];
  char destdir[strlen(argv[2])+1];
  strcpy(filename,argv[1]);
  strcpy(destdir,argv[2]);

  int maxOutLines = -1;
  if(argc >= 4) maxOutLines = atoi(argv[3]);

  int barrelOnly = 0; // flag
  bool selectBX  = false;
  if(argc >= 5) 
  {
    std::cout << "writing only barrel muons and discarding vindex" << std::endl;
    int flag = atoi(argv[4]);
    barrelOnly  = flag;
    selectBX = flag == 2 ? true:false; // if flag == 2, then selectBX is true and barrelOnly is also true
    if(selectBX) std::cout << "INFO: selecting only barrel muons in the abort gap " << std::endl;
  }
  int offset = 0;
  if(argc >= 6) offset = atoi(argv[5]);

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

  // create CSV file
  char txtfilename[strlen(destdir)+strlen(filename)+1];
  char txtfilenamedimuon[strlen(destdir)+strlen(filename)+1];
  sprintf(txtfilename,"%s/scout_%s_%06d.txt",destdir,  runnumber, ordinal);
  sprintf(txtfilenamedimuon,"%s/scout_%s_%06d.dimuon.txt",destdir,  runnumber, ordinal);
  std::ofstream   out(txtfilename, std::ios_base::trunc); // don't append, any contents that existed in the file before it is open are discarded.
  std::ofstream   outdimuon(txtfilenamedimuon, std::ios_base::trunc); // don't append, any contents that existed in the file before it is open are discarded.
  fprintf(stderr,"created txt file %s\n", txtfilename);

  // writing the header of the CSV file
  out <<        "orbit"   << "," << "bx"    
      << "," << "phi"     << "," << "eta"
      << "," << "pt"      << "," << "charge";

  if(!barrelOnly)   
  out << "," << "vindex"; 

  out << std::endl;
  // -- endOf CSV's header

  // writing the header of the outdimuon CSV
  
	outdimuon << "orbit"                          << "," ;
        outdimuon << "bx"                             << "," ;
        outdimuon << "nMu"                            << "," ;
        outdimuon << "nMu5"                           << "," ;
        outdimuon << "nMu10"                          << "," ;
        outdimuon << "mass"                           << "," ;
        outdimuon << "massExt"                        << "," ;
        outdimuon << "pt"                             << "," ;
        outdimuon << "ptExt"                          << "," ;
        outdimuon << "Y"                              << "," ;
        outdimuon << "YExt"                           << "," ;
        outdimuon << "pt1"                            << "," ;
        outdimuon << "pt2"                            << "," ;
        outdimuon << "eta1"                           << "," ;
        outdimuon << "eta2"                           << "," ;
        outdimuon << "ch1"                            << "," ; 
        outdimuon << "ch2"                            ;
        outdimuon << std::endl;






  // --- endOf outdimuon CSV

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
  uint64_t discarded = 0;
  fprintf(stderr,"opening file %s\n",filename);
  
  FILE *f=fopen(filename,"r");
  block bl;

  int eventcount = 0;
  int ibytes = 0;
  uint32_t header;
  float pt_cut= 1.5;
  uint32_t qual_cut = 12;
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
    if (bl.bx>0xf0000000) bx= bl.bx-0xf0000000; 
    orbit = bl.orbit;

    std::vector<L1Muon> L1Muons;
    std::vector<L1Muon> L1MuonsExt; // extrapolated to the vertex

    if(!goodBX(bx) && selectBX ) continue; // keep only the bxs that are logic of selectBX function

    for(unsigned int i=0;i<mAcount+mBcount; i++)
    {
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

      bool endcapMu = false; 
      if(index<36 || index>70) endcapMu = true;

      if(barrelOnly && endcapMu) continue; // don't fill endcapMu if we have barrelOnly = true

      L1Muon muonCandidate;
      muonCandidate.SetPtEtaPhiM(pt, eta, phi, 0.1057);
      muonCandidate.charge  = chrg;
      muonCandidate.index   = index;
      muonCandidate.qual    = qual;
      L1Muons.push_back(muonCandidate);

      L1Muon muonCandidateExt;
      muonCandidateExt.SetPtEtaPhiM(pt, etaext, phiext, 0.1057);
      muonCandidateExt.charge  = chrg;
      muonCandidateExt.index   = index;
      muonCandidateExt.qual    = qual;
      L1MuonsExt.push_back(muonCandidateExt);
      
      vpt.push_back(pt);
      veta.push_back(eta);
      vphi.push_back(phi);
      vetap.push_back(etaext);
      vphip.push_back(phiext);
      vcharge.push_back(chrg);
      vindex.push_back(index);
      vqual.push_back(qual);
    }

    if(vpt.empty() && !barrelOnly)
    { 
      vpt.push_back(-99);
      veta.push_back(-99);
      vphi.push_back(-99);
      vetap.push_back(-99);
      vphip.push_back(-99);
      vcharge.push_back(-99);
      vindex.push_back(-99);
      vqual.push_back(-99);
      discarded++;
    }

    if(vpt.size()>=2) {  // write only lines in case of dileptons
    
        std::sort(L1Muons.begin(), L1Muons.end(), [](const L1Muon a, const L1Muon b){return a.Pt() > b.Pt();}); 
        std::sort(L1MuonsExt.begin(), L1MuonsExt.end(), [](const L1Muon a, const L1Muon b){return a.Pt() > b.Pt();}); 



        int NM5  = 0; // nmuons with greater than 5  GeV pt
        int NM10 = 0; // nmuons with greater than 10 GeV pt
        for(auto a: L1MuonsExt) // write the extrapolated muons
        {
            if(a.Pt()>5) NM5++;
            if(a.Pt()>10) NM10++;

             out <<        orbit       << "," << bx    
                 << "," << a.Phi()     << "," << a.Eta()
                 << "," << a.Pt()      << "," << a.charge;

              if(!barrelOnly)   
             out << "," << a.index; 

             out << std::endl;
        }
        
	outdimuon << orbit                                 << "," ;
        outdimuon << bx                                    << "," ;
        outdimuon << L1Muons.size()                        << "," ;
        outdimuon << NM5                                   << "," ;
        outdimuon << NM10                                  << "," ;
        outdimuon << (L1Muons[0] + L1Muons[1]).M()         << "," ;
        outdimuon << (L1MuonsExt[0] + L1MuonsExt[1]).M()   << "," ;
        outdimuon << (L1Muons[0]+L1Muons[1]).Pt()          << "," ;
        outdimuon << (L1MuonsExt[0] + L1MuonsExt[1]).Pt()  << "," ;
        outdimuon << (L1Muons[0]+L1Muons[1]).Y()           << "," ;
        outdimuon << (L1MuonsExt[0] + L1MuonsExt[1]).Y()   << "," ;
        outdimuon << L1Muons[0].Pt()                       << "," ;
        outdimuon << L1Muons[1].Pt()                       << "," ;
        outdimuon << L1Muons[0].Eta()                      << "," ;
        outdimuon << L1Muons[1].Eta()                      << "," ;
        outdimuon << L1Muons[0].charge                     << "," ; 
        outdimuon << L1Muons[1].charge                     ;
        outdimuon << std::endl;
     
        
    }

  }// end of while loop
  fclose(f);

  std::cout << "number of empty (bx, orbits) = " << discarded << std::endl;
    

  perror("blah");
    
}
