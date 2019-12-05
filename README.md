# 40 MHz datascouting analysis framework (CMS@CERN)   

## Introduction 
Searching for the Higgs boson and beyond the standard model phenomena at the [LHC](https://home.cern/science/accelerators/large-hadron-collider) demands many proton-proton (pp) collision events at the highest possible achievable energy (13 TeV).

By design, the LHC produces far too many collisions to be recorded on disk. With an LHC bunch crossing [(BX)](https://lhc-machine-outreach.web.cern.ch/lhc-machine-outreach/beam.htm) rate of 40MHz (1 BX per 25ns) and using the latest greatest technologies (FPGAs + computing farms) available upon its construction, the [CMS experiment](https://cms.cern/detector) can “briefly look" at all BX's, but preserve (i.e., [trigger](https://cms.cern/detector/triggering-and-data-acquisition)) for later analysis only a tiny fraction of them, ~2.5 out of every million (10^6) of BX's. Certain rules have to be applied in order to prevent CMS from attempting to record too many pp collisions in short time, namely no more than 1 trigger-accept in 3 BX’s, 2 in 25, 3 in 100, 4 in 240. 

At least these all were true once upon a time. In practice, CMS collaboration managed to improve the situation by a factor of ~2, in the late times of the (Run2)[https://project-hl-lhc-industry.web.cern.ch/content/project-schedule]. Still, there is no waterproof assurance that we don’t miss from seeing interesting events that are produced during the pp collisions but the trigger (event selection) logic has not been programmed to be aware and anticipate for. In particular, slowly evolving phenomena that extend in time in more than 1 BX (>25 ns), would be nearly next to impossible to trigger on with the present architecture. In addition, final states in regions of the phase space where the background is large are technically limited by the available bandwidth of the trigger. 

The 40 MHz scouting, is a novel CMS R&D project, having as ambition to bypass the present architectural limitations enabling the recording of ALL of the collision events without the need of a trigger-accept, at a price of reduced resolution on the recorded “pictures”. In a pilot run during 2018, the 40 MHz scouting system recorded for first time collision and cosmic rays data. In this first test-run, only the Muon subsystem of the CMS detector participated in the scouting. The full blown production-level system is envisioned to be ready for (phase-II)[https://project-hl-lhc-industry.web.cern.ch/content/project-schedule] (>2027). Further pilot test-runs are scheduled for (Run 3)(https://lhc-commissioning.web.cern.ch/lhc-commissioning/schedule/LHC-long-term.htm).

## This project is about ...
analyzing the first-ever-recorded scouting data and searching for regularities and irregularities therewithin. The size of the sample could  easily become unmanageable (**big data**) if not smart slicing and selection is applied. Industrial standards in terms of computational tools will be used for the analysis of the 2018 scouting data.  

## Experimental setup
<img src = "http://theofil.web.cern.ch/theofil/dataScout/dscout.png"></img>

##  Plan
* Analyze triggerless 40MHz scouting data recorded in 2018
    * Rate vs time (time invariance ? day/night)
    * Correlations among BX's
        * Display evidence of a signal == a cosmic muon traversing the CMS detector with ~speed of light in ~about 50ns, with its trajectory recorded (split) in neighboring BX's
    * Optimize selection of Run/BX/orbit to avoid (or select) collisions, discard problematic runs
    * Web scraping of information from logbooks, CMS run registy, lpc
    * Search for muons signals coming from deep earth, place limits on their rate 
* publish in GitHub polished analysis code to be used in future scouting data (expected in ~2021)
* further develop and polish the software (C++) used to read & select from RAW data files
* document a CMS internal note (latex)

## Download example files with cosmic rays 
Example data files from run 326790 can be downloaded [here](https://theofil.web.cern.ch/theofil/dataScout/index.php).
* **scout_326790_000000_trunc.txt**  -> contains 10k records that have been processed with a custom-made RAW data reader [selectBX.cc](https://github.com/theofil/dscout/blob/master/readBinary/selectBX.cc) capable of selecting BX's that lay in the LHC's abort gap
* **scout_326790_000000.monitor.txt** -> summarizes the number of aggregated records per BX for run 326790 and is the output of the custom made RAW data monitor [runMonitor](https://github.com/theofil/dscout/blob/master/readBinary/runMonitor.cc)

## Analysis
The analysis will be carried out in two steps:
* Distill data from the RAW CMS data-acquision (.dat) files, e.g., save all muons detected in barrel of CMS for user-specified time intervals. Massive job submission will be carried out by [HTCondor batch system](http://information-technology.web.cern.ch/services/fe/lxbatch/howto/quickstart-guide-htcondor) at [lxplus (linux servers)](http://information-technology.web.cern.ch/services/lxplus-service).
* Analyze the distilled data in jupiter notebooks, using python3 and [SWAN service](https://swan.web.cern.ch/).
An example jupiter notebook could be found [here](https://github.com/theofil/dscout/blob/master/analysis/exampleAnalysis.ipynb)

## Keywords  
* LHC beam structure
* CMS detector, muons subsystem
* cosmic muon  (asynchronously) triggered data vs triggerless muon data from 40MHz scouting
* Data analysis 
   * selection, cleaning, slicing, issues with real data
   * rate, histograms, correlations
* Industrial standards (python3) used in big data/machine learning 
   * pandas
   * numpy 
   * scikit
   * jupyter notebooks
   * web scarping/log collection 
* C/C++ 
   * standard template library 
   * binary shifts & masks
* CERN's HTCondor batch system
* Cloud with [SWAN service](https://swan.web.cern.ch/)
* git, github 
* Latex
* Linux
