import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import pandas as pd
import seaborn as sns
import os
import datetime

class RUN:
    timePerOrbit = 110 * 24.95

    def __init__(self, dataFrame):

        self.df = dataFrame
        self.events = len(self.df)

        self.orbitMin = self.df['orbit'][:1].values[0]
        self.orbitMax = self.df['orbit'][-1:].values[0]
        self.nOrbits = self.orbitMax - self.orbitMin
        self.rectime = self.nOrbits * RUN.timePerOrbit  # in ns
        self.rectimeSECONDS = self.nOrbits * RUN.timePerOrbit * 10 ** (-9)  # in s

        if (self.df['run'][0] == self.df['run'][-1:].values[0]):  # This code is not intended for use on merged runs
            self.run = self.df['run'][0]
            TimeLogPath = "./runsDateTimeLog.txt"
            TimeLog = pd.read_csv(TimeLogPath, delimiter=', ', engine='python')

            for i in range(len(TimeLog['run'])):
                if TimeLog['run'][i] == self.run:
                    index = i
                    break

            duration = TimeLog['duration'][index]
            self.tStart = TimeLog['time_start'][index]
            self.tEnd = TimeLog['time_end'][index]
            h, m, s = duration.split(":")
            durationSecs = datetime.timedelta(hours=int(h), minutes=int(m), seconds=int(s)).total_seconds()
            #       --------------------------
            self.duration = durationSecs
            self.naiveRate = self.events / durationSecs  # in seconds

    #       --------------------------

    def rate(self, TU=1):#, save_name = '', plot = False):
    
    
    
        df = self.df['orbit']
        
        timePerOrbit = 110*24.95
        # print('timePerOrbit %2.1f ns '%timePerOrbit)

        timeUnit = TU*1e9 # desired time unit in nanoseconds
        orbitsPerTimeUnit = timeUnit/timePerOrbit
        # print('orbitsPerTimeUnit = %2.3f'%orbitsPerTimeUnit)

        orbitMin  = df[:1].values[0]
        orbitMax  = df[-1:].values[0] 
        # rectime   = (orbitMax-orbitMin)*timePerOrbit # in ns
        # print('orbitMin %d orbitMax %d'%(orbitMin, orbitMax))
        # print('nOrbits %d '%(orbitMax - orbitMin))
        # print('total recording time %2.4f ns (~= %2.1f s)' %(rectime, rectime*1.e-9))

        bins=int((orbitMax-orbitMin)/orbitsPerTimeUnit) 
        # print("bins = %d" %bins)
        # timePerBin = rectime/bins
        
        muons=np.zeros(bins)
        sclice=np.array([orbitMin+ ((orbitMax-orbitMin)/bins)*i for i in range(bins+1)])

        # Count the events in the first orbit = orbitMin
        for orbit in df:
            if(orbit > orbitMin): break
            if(orbit == orbitMin): muons[0] += 1

        df = df[int(muons[0]):] # We already counted the first orbit
        i = 0
        for orbit in df:
            while not (orbit > sclice[i] and orbit <= sclice[i + 1]) and i < bins-1: # Note the <= in 'orbit <= slicePlusOne[i]'. 
                i = i + 1                                                              # It is needed to capture the last orbit.
            muons[i] += 1
            
        # # Maximum Likelihood Estimation of Poisson parameter
        # Nmuons = int(muons.sum()) 
        # Ninterval = len(muons) # How many time intervals

        # mu = muons.mean()
        # med = np.median(muons)
        # std = muons.std()
        
        
        # if plot:
        #     plt.rc('text', usetex=True)
        #     display = r"\begin{eqnarray*} \\" + r"\mathrm{Mean}" +r"&=& {:.2f}\\".format(mu) + r"\mathrm{Median}"+ r"&=& {:.0f} \\ ".format(med) + r"\mathrm{Std}"+r"&=& {:.2f} \\ ".format(sigma)+ r"\end{eqnarray*}"
            

            
        #     plt.figure(figsize=(8,8))

        #     muons = pd.Series(muons)
        #     counts = muons.value_counts().sort_index()

        #     plt.errorbar(counts.index, counts, xerr = None, yerr = np.sqrt(counts), fmt = 'o')
        #     plt.xlabel(r"$\mathrm{nMuons}$"+r"$/ {:2.1f} s$".format(timePerBin*1.e-9), fontsize = 20) # latex can be entered in the label's string
        #     plt.ylabel(r"$\mathrm{Frequency}$", fontsize = 20)

        #     plt.text(0.84, 0.9, display,fontdict= {'fontsize': 20}, horizontalalignment='center',  verticalalignment='center', transform=plt.gca().transAxes)
            
        #     # print('--------------------------------')
        #     # print('mean = {:.2f}' .format(mu))
        #     # print('median = {:d}'.format(int(med)))
        #     # print('sample std = {:.2f}'.format( sigma))
        #     #print('Gaussian 99% Confidence Interval = [{:.2f} , {:.2f} ]'.format(lower, upper))
        #     if len(save_name)>0: plt.savefig('./Plots/RatePlots/{}'.format(save_name))
        #     plt.show()

        
        return np.array(muons)
        

    def plotPhi(self, bins=30):

        phiticks = [0.5 * np.pi * i for i in range(-2, 3)]
        # plt.rc('font', size=22)
        # plt.figure(figsize=(8, 8))
        plt.hist(self.df['phi'], bins=bins, ec='black', histtype='stepfilled')
        plt.xticks(phiticks)
        plt.xlabel(r'$\phi$ [radians]')
        plt.ylabel('Muons')
        plt.title('Run [{}], Duration = {:.2f} min '.format(self.run, self.duration / 60), size=30)
        # plt.show()

    def plotOrbit(self, bins=100):

        # plt.rc('font', size=22)
        # plt.figure(figsize=(8, 8))
        plt.hist(self.df['orbit'], bins=bins, ec='black', histtype='stepfilled')
        plt.xlabel('Orbit', size=30)
        plt.ylabel('Muons', size=30)
        plt.xticks(size=20)
        plt.yticks(size=20)
        plt.title('Run [{}], Duration = {:.2f} min '.format(self.run, self.duration / 60), size=30)
        # plt.show()


def pair(df, x):
    dfns = df.shift(-x)
    dfps = df.shift(+x)

    logicOS1 = (df.bx == dfns.bx - x) & (df.orbit == dfns.orbit) & (df.charge * dfns.charge < 0)
    OS1 = df[logicOS1].copy()

    logicOS2 = (df.bx == dfps.bx + x) & (df.orbit == dfps.orbit) & (df.charge * dfps.charge < 0)
    OS2 = df[logicOS2].copy()

    OS1 = OS1.reset_index()
    OS2 = OS2.reset_index()

    OS = OS1.merge(OS2, left_on=OS1.index, right_on=OS2.index, suffixes=('_in', '_out'))

    logicSS1 = (df.bx == dfns.bx - x) & (df.orbit == dfns.orbit) & (df.charge * dfns.charge > 0)
    SS1 = df[logicSS1].copy()

    logicSS2 = (df.bx == dfps.bx + x) & (df.orbit == dfps.orbit) & (df.charge * dfps.charge > 0)
    SS2 = df[logicSS2].copy()

    SS1 = SS1.reset_index()
    SS2 = SS2.reset_index()

    SS = SS1.merge(SS2, left_on=SS1.index, right_on=SS2.index, suffixes=('_in', '_out'))

    # slim the dataframes
    def slimDataFrame(df):
        df = df.drop(['run_in', 'index_in', 'index_out', 'key_0', 'orbit_out'], axis=1)
        df = df.rename(columns={'run_out': 'run', 'orbit_in': 'orbit'})
        if False: df = df.set_index(['run', 'orbit', 'bx_in'])
        return df

    OS = slimDataFrame(OS)
    SS = slimDataFrame(SS)
    return (OS, SS)