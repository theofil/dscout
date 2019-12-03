# Analysis framework for the LHC data scouting @ 40MHz 

## Introduction
Searching for the Higgs boson and beyond the standard model phenomena at the LHC, demands many proton-proton (pp) collision events at the highest possible achievable energy (13 TeV).

By design, the LHC produces far too many collisions to be recorded on disk. With an LHC bunch crossing (BX) rate of 40MHz (1 BX per 25ns) and using the latest greatest technologies (FPGAs) available upon its construction, the CMS experiment can “briefly look" at all BX's, but preserve (i.e., trigger) for later analysis only a tiny fraction of them, ~2.5 out of every million (10^6) of BX's. Certain rules have to be applied in order to prevent CMS from attempting to record too many pp collisions in short time, namely no more than 1 trigger-accept in 3 BX’s, 2 in 25, 3 in 100, 4 in 240. 

At least these all were true once upon a time. CMS detector upgrades managed to improve the situation by a factor of ~2, in the late times of the Phase-I (2009-2018). Still, there is no waterproof assurance that we don’t miss from seeing interesting events that are produced during the pp collisions but the trigger (event selection) logic has not been programmed to be aware and anticipate. In particular, slowly evolving phenomena that extend in time in more than 1 BX (>25 ns), would be nearly next to impossible to trigger on with the present architecture. In addition, final states in regions of the phase space where the background is large are technically limited by the available bandwidth of the trigger. 

The 40 MHz scouting, is a novel proposal which attempts to bypass the present architectural limitations enabling the recording of ALL the collision events, at a price of reduced resolution on the recorded “pictures”. In a pilot running of the 40 MHz scouting system during 2018, CMS has collected some LHC collision and cosmic rays data. Due technical reasons, only the Muon subsystem of the CMS detector participated in the data scouting process.

This project is about analyzing the scouting data and search for patterns indicating regularities and irregularities within these never-studied-before data.


