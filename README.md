# Analysis framework for the LHC data scouting @ 40MHz 

## Introduction 
Searching for the Higgs boson and beyond the standard model phenomena at the LHC, demands many proton-proton (pp) collision events at the highest possible achievable energy (13 TeV).

By design, the LHC produces far too many collisions to be recorded on disk. With an LHC bunch crossing (BX) rate of 40MHz (1 BX per 25ns) and using the latest greatest technologies (FPGAs) available upon its construction, the CMS experiment can “briefly look" at all BX's, but preserve (i.e., trigger) for later analysis only a tiny fraction of them, ~2.5 out of every million (10^6) of BX's. Certain rules have to be applied in order to prevent CMS from attempting to record too many pp collisions in short time, namely no more than 1 trigger-accept in 3 BX’s, 2 in 25, 3 in 100, 4 in 240. 

At least these all were true once upon a time. CMS detector upgrades managed to improve the situation by a factor of ~2, in the late times of the Phase-I (2009-2018). Still, there is no waterproof assurance that we don’t miss from seeing interesting events that are produced during the pp collisions but the trigger (event selection) logic has not been programmed to be aware and anticipate. In particular, slowly evolving phenomena that extend in time in more than 1 BX (>25 ns), would be nearly next to impossible to trigger on with the present architecture. In addition, final states in regions of the phase space where the background is large are technically limited by the available bandwidth of the trigger. 

The 40 MHz scouting, is a novel proposal which attempts to bypass the present architectural limitations enabling the recording of ALL the collision events, at a price of reduced resolution on the recorded “pictures”. In a pilot running of the 40 MHz scouting system during 2018, CMS has collected some LHC collision and cosmic rays data. Due technical reasons, only the Muon subsystem of the CMS detector participated in the data scouting process. The full blown 40 MHz scouting system is envisioned to be ready for phase-II (>2023). Further pilot runs of this system are scheduled for the LHC Run 3 [LHC plan](https://lhc-commissioning.web.cern.ch/lhc-commissioning/schedule/LHC-long-term.htm).

This project is about analyzing the scouting data and search for patterns indicating regularities and irregularities therewithin. 

##  Plan
* analyze triggerless 40MHz scouting recorded by CMS in 2018
    * study of the rate vs time 
    * study of correlations among BX's
        * display evidence of the signal == a cosmic muon traversing the CMS detector with ~speed of light in ~about 50ns, with its trajectory data recorded (split) in neighboring BX's
    * optimize selection of Run/BX/orbit to avoid (or select) collisions, discard problematic runs
    * master in web scarping of information from logbooks, CMS run registy, lpc
    * search for muons signals coming from deep earth, place limits on their rate 
    * study signal’s rate variation vs time (time invariance ? day/night)
* publish in GitHub polished analysis code to be used in future scouting data (expected in ~2021)
* further develop and polish the software (C++) used to read & select from RAW data files
* document a CMS internal note (latex)

## Example sample with cosmic rays 
Example data files from run 326790 can be downloaded [here](https://theofil.web.cern.ch/theofil/dataScout/index.php).
* **scout_326790_000000_trunc.txt**  -> contains 10k records that have been processed with a custom-made RAW data reader [selectBX.cc](https://github.com/theofil/dscout/blob/master/readBinary/selectBX.cc) capable of selecting BX's that lay in the LHC's abort gap
* **scout_326790_000000.monitor.txt** -> summarizes the number of aggregated records per BX for run 326790 and is the output of the custom made RAW data monitor [runMonitor](https://github.com/theofil/dscout/blob/master/readBinary/runMonitor.cc)


## Learning goals in keywords  
* LHC, orbits, beam structure
* CMS detector, muons in CMS
* cosmic muon  (asynchronously) triggered data vs triggerless muon data from 40MHz scouting
* data analysis (selection, cleaning, issues with real data)
* probability, rate, histograms, correlations
* modern tools used in big data/machine learning (pandas, numpy, scikit)
* CERN's HTCondor batch system
* web scarping 
* git, python, latex, C++
