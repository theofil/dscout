import numpy as np
import pandas as pd
# import matplotlib.pyplot as plt
import os
import subprocess
import bz2
import glob
import time 
import datetime
from shutil import copy2, rmtree
from scoutFunctions import *





#########################################
##                                     ##
##    SECTION 1: DATA CLEANING         ##
##                                     ##
#########################################

# Create a Tree Structure

if not os.path.exists('./data/RemovedFiles'):
        os.makedirs('./data/RemovedFiles')
        
writePath  = ['./data/allRuns/', './data/goodRuns', './data/badRuns', './data/2LegAll',
              './data/2LegGood', './data/2LegBad']
twolegPath = ['DBx1/SS/', 'DBx1/OS/', 'DBx2/SS/', 'DBx2/OS/', 'DBx3/SS/', 'DBx3/OS/']
runPath = ['monitor/', 'csv/']
for path in writePath:
    if not os.path.exists(path):
        os.makedirs(path)
    else: 
        rmtree(path)
        os.makedirs(path)
    if path in writePath[0:3]:
        for rPath in runPath:
            endPath =  os.path.join(path, rPath)
            os.makedirs(endPath)
    if path in writePath[3:6]:
        for tlPath in twolegPath:
            endPath =  os.path.join(path, tlPath)
            os.makedirs(endPath)
    



# Clean data files
path = './data/hiion/' 
files = os.listdir(path)
files = [f for f in files if 'monitor' not in f]

# getrunNumber from string e.g., int('/data/hiion/scout_326676_000000.txt'.split('_')[1]
getRun = lambda x: int(x.split('_')[1])


default_line = 'orbit,bx,phi,eta,pt,charge'

print('Working Directory:')
print(os.path.dirname(os.path.abspath(files[0]))) # print path for safety

files_to_remove = []

for i in range(len(files)):
    
    with open(os.path.join(path, files[i]), 'r') as file:
        
        file.seek(0)
        x=file.read(len(default_line + '\n'))
        file.seek(0)
        y=file.read(len(default_line + '\n')+1)
        file.seek(0)
        
        if x.strip("\n") == y.strip("\n"):              #checks if there is only the default line 'orbit,bx,phi,eta,pt,charge\n'
            
            files_to_remove.append(files[i])
            monitor = files[i].split('.')[0]+'.monitor.txt'
            files_to_remove.append(monitor)
            print(files[i] + ' is EMPTY')
            print(monitor + ' is EMPTY')
            
        elif i>0 and getRun(files[i]) == getRun(files[i-1]):

            with open(os.path.join(path, files[i-1]), 'a') as oldfile:
                for line in file:
                    if line.strip("\n") != default_line :
                        oldfile.write(line)

            monitor = files[i].split('.')[0]+'.monitor.txt'
            oldmonitor = files[i-1].split('.')[0]+'.monitor.txt'
            print(monitor)
            print(oldmonitor)
            newMonitor = []
            with open(os.path.join(path, oldmonitor), 'r') as oldmonit, open(os.path.join(path, monitor), 'r') as monit:
                for oldline, line in zip(oldmonit, monit):
                    x = int(line)
                    y = int(oldline)
                    newMonitor.append(x + y)
            print(len(newMonitor))
            with open(os.path.join(path, oldmonitor), 'w') as oldmonit:
                for value in newMonitor:
                    oldmonit.write(str(value)+'\n')


            files_to_remove.append(files[i])
            
            files_to_remove.append(monitor)
            print(files[i] + ' is a continuation of another file of the same run. ')
            print(monitor + ' is a continuation of another file of the same run. ')



destination = './data/RemovedFiles/'
for f in files_to_remove:
        if len(files_to_remove)>0:
            if os.path.exists(os.path.join(path, f)):
                copy2(os.path.join(path, f), destination)
                os.remove(os.path.join(path, f))
                print('Moving  '+ f + '  to the RemovedFiles Folder')
            else:
                print("The "+ f + "file does not exist")


# Write clean data to the tree


# list to hold dataframes
dfList = []
runIndex= []

path = './data/hiion/' 
files = os.listdir(path)
files = [f for f in files if 'monitor' not in f]

getRun = lambda x: int(x.split('_')[1])

for file in files:
    filepath = path+file
    df = pd.read_csv(filepath)
    
    # add a column with the run number
    df['run'] = getRun(file) 
    dfList += [df]
    
    #make in index with the run numbers
    runIndex += [getRun(file)]

# ---------------------------------------------------------------------------------------------
# OLD CERTIFICATION
# ---------------------------------------------------------------------------------------------

# #Fix the runs where the orbit number resets (regard only muons after the reset)
# dfs = []
# for df in dfList:
#     reset = 0
#     if not df['orbit'].is_monotonic:
#         for i in range(1,len(df)):
#             if df.loc[i-1, 'orbit']>df.loc[i, 'orbit']:
#                 reset = i
#                 break
#     tmp = df.loc[reset:]
#     tmp.reset_index(drop=True, inplace=True) 
#     dfs.append(tmp)            
# ---------------------------------------------------------------------------------------------




#Fix the runs where the orbit number resets 
dfs = []
runsWithReset = []
for df in dfList:
    reset = 0
    # fix an issue of the continuation of run 326587 which was split into two ordinal files which we merged together
    if df['run'][0] == 326587: 
        gluepoint = 81801667
        glueindex = 43774
        df.loc[glueindex:, 'orbit'] += gluepoint+1 - df.loc[glueindex, 'orbit'] 

    if not df['orbit'].is_monotonic:
        runsWithReset.append(df['run'][0])
        for i in range(1,len(df)):
            if df.loc[i-1, 'orbit']>df.loc[i, 'orbit']:
                tmp = i-1
                break
        df.loc[tmp+1:, 'orbit'] += df.loc[tmp, 'orbit']
        df['OrbitReset'] = np.tri(1, len(df), tmp, dtype = int)[0].tolist()
    dfs.append(df)
    

for df in dfs:
    name = str(df['run'][0])+".txt"
    df.to_csv("./data/allRuns/csv/"+name, index = False) 
    for i in range(3):
        OS, SS = pair(df,i+1)
        OS.to_csv("./data/2LegAll/DBx{}/OS/".format(i+1)+name, index = False)
        SS.to_csv("./data/2LegAll/DBx{}/SS/".format(i+1)+name, index = False) 



# Write clean data to the tree


# list to hold dataframes
monitorList = []
runIndex= []

path = './data/hiion/' 
files = os.listdir(path)
files = [f for f in files if 'monitor' in f]

getRun = lambda x: int(x.split('_')[1])

for file in files:
    filepath = path+file
    df = pd.read_csv(filepath)
    
    # add a column with the run number
    
    monitorList += [df]
    
    #make in index with the run numbers


#Fix the runs where the orbit number resets (regard only muons after the reset)

for run, df in zip(dfs, monitorList):
    name = str(run['run'][0])+".monitor.txt"
    df.to_csv("./data/allRuns/monitor/"+name, index = False) 


#########################################
##                                     ##
##    SECTION 2: DATA CERTIFICATION    ##
##                                     ##
#########################################



path = './data/allRuns/csv/'
def DurationFilter(path, cut):
    
    files = os.listdir(path)
    files = [f for f in files]

    # list to hold runs
    runList = []
#     rate = []
    duration = []
#     rectime = []
    runNo = []
    rejectedList = []
    for file in files:
        filepath = path+file
        df = pd.read_csv(filepath)
        runObj = RUN(df)
        
        if runObj.duration > cut:
            runList.append(runObj)
        else: rejectedList.append(runObj)
    return runList, rejectedList


runs, rejected = DurationFilter(path, 600)
print("Number of Runs longer than 10 minutes: {}".format(len(runs)))
print("Number of Runs shorter than 10 minutes: {}".format(len(rejected)))


# After reviewing the runs, we classify our runs:
badrunNo = [326262, 326303, 326586, 326859, 326884, 326887, 326888, 326965, 327022, 327125, 327147, 327148, 327378, 327402, 327403, 327424, 327430, 327431, 327455, 327462, 327466, 327488, 327489]
badruns = [run for run in runs if run.run in badrunNo]
goodruns = [run for run in runs if run.run not in badrunNo]

# Write good and bad runs to the Tree
        
for run in goodruns:
    name = str(run.run)+".txt"
    run.df.to_csv("./data/goodRuns/csv/"+name, index = False) 
    for i in range(3):
        OS, SS = pair(run.df,i+1)
        OS.to_csv("./data/2LegGood/DBx{}/OS/".format(i+1)+name, index = False)
        SS.to_csv("./data/2LegGood/DBx{}/SS/".format(i+1)+name, index = False) 
    
for run in badruns:
    name = str(run.run)+".txt"
    run.df.to_csv("./data/badRuns/csv/"+name, index = False) 
    for i in range(3):
        OS, SS = pair(run.df,i+1)
        OS.to_csv("./data/2LegBad/DBx{}/OS/".format(i+1)+name, index = False)
        SS.to_csv("./data/2LegBad/DBx{}/SS/".format(i+1)+name, index = False) 
    

# Write good and bad monitor files to the Tree
path = './data/allRuns/monitor/'
    
files = os.listdir(path)
files = [f for f in files]

# list to hold monitor files
goodmonitor = []
goodmonitorNo = []
badmonitor = []
badmonitorNo = []
goodrunNo = [run.run for run in runs if run.run not in badrunNo]
for file in files:
    filepath = path+file
    df = pd.read_csv(filepath)
    number = int(file.split('.')[0])
    if number in badrunNo:
        badmonitor.append(df)
        badmonitorNo.append(number)
    if number in goodrunNo: 
        goodmonitor.append(df)
        goodmonitorNo.append(number)
    


for i in range(len(goodmonitorNo)):
    name = str(goodmonitorNo[i])+".txt"
    goodmonitor[i].to_csv("./data/goodRuns/monitor/"+name, index = False) 
    
for i in range(len(badmonitorNo)):
    name = str(badmonitorNo[i])+".txt"
    badmonitor[i].to_csv("./data/badRuns/monitor/"+name, index = False) 


print('Total Runs: {}\nRuns longer than 10 mins: {}\nGood Runs: {}\nBad Runs: {}'.format(len(files), len(runs), len(goodruns), len(badruns)))


# Latex table for good runs
print('LATEX TABLE OF GOOD RUNS\n\n\n')
for i in range(int(len(goodruns))):
    print("{}&{}&{:.2f}&{:.2f} \\\ ".format(goodruns[i].run, goodruns[i].events, goodruns[i].duration/60, goodruns[i].rectimeSECONDS/60))


badrunNo = [326262, 326303, 326586, 326859, 326884, 326887, 326888, 326965, 327022, 327125, 327147, 327148, 327378, 327402, 327403, 327424, 327430, 327431, 327455, 327462, 327466, 327488, 327489]
# Latex table for bad runs
print('LATEX TABLE OF BAD RUNS\n\n\n')
for i in range(int(len(badruns))):
    print("{}&{}&{:.2f}&{:.2f} \\\ ".format(badruns[i].run, badruns[i].events, badruns[i].duration/60, badruns[i].rectimeSECONDS/60))

# Compare run duration as given by the CMS registry and our effective recorded abort gap time

# GOOD_rectime = np.array([run.rectimeSECONDS for run in goodruns])
# GOOD_duration = np.array([run.duration for run in goodruns])
# GOOD_timeRatio = GOOD_duration/GOOD_rectime
# BAD_rectime = np.array([run.rectimeSECONDS for run in badruns])
# BAD_duration = np.array([run.duration for run in badruns])
# BAD_timeRatio = BAD_duration/BAD_rectime

# # Duration - Rectime PLOT
# goodrunNo = [run.run for run in runs if run.run not in badrunNo]
# plt.figure(figsize = (20,10))
# plt.scatter(GOOD_rectime, GOOD_duration, c= 'blue', label = 'Good Runs')
# plt.scatter(BAD_rectime, BAD_duration, c = 'red', label = 'Bad Runs')
# plt.ylabel('Duration of Run [seconds]', fontsize = 30)
# plt.xlabel('Recorded abort gap time [seconds]', fontsize = 30)
# xmin, xmax, ymin, ymax = plt.axis()
# x = np.array([i for i in range(int(xmin), int(xmax))])
# plt.plot(x, 3565/110 * x , c='black', label = 'y=x*3565/110')
# # plt.xscale('log')
# # plt.yscale('log')
# plt.legend(fontsize = 30)
# plt.yticks(fontsize = 20)
# plt.xticks(fontsize = 20)
# # plt.savefig('./Plots/rectime-duration.pdf')
# plt.show()


# # --------------------------------------------------------------------------------------------

# # Time Ratio PLOT
# goodrunNo = [run.run for run in runs if run.run not in badrunNo]
# plt.figure(figsize = (20, 10))
# plt.rc('text', usetex=True)

# x = np.array([i for i in  range(len(goodrunNo))])
# y = GOOD_timeRatio

# plt.scatter(x, y, s = 100, label = 'Good Runs', color = 'blue')

# xb = np.array([i+len(GOOD_timeRatio) for i in  range(len(badrunNo))])
# yb = BAD_timeRatio

# plt.scatter(xb, yb, s = 100, label = 'Bad Runs', color = 'red')

# xmin, xmax, ymin, ymax = plt.axis()

# for i in range(len(GOOD_timeRatio)):
#     plt.vlines(x[i], min(3565/110 ,GOOD_timeRatio[i]), max(3565/110 ,GOOD_timeRatio[i]), colors = 'black', linestyles = "dotted")

# for i in range(len(BAD_timeRatio)):
#     plt.vlines(xb[i], min(3565/110 ,BAD_timeRatio[i]), max(3565/110 ,BAD_timeRatio[i]), colors = 'black', linestyles = "dotted")

# plt.hlines(3565/110, xmin, xmax, colors = 'black', label = "Expected Value = 3565/110")

# plt.xlabel('Run Number',  fontsize = 50)
# plt.ylabel(r'$\frac{\mathrm{duration}}{\mathrm{rectime}}$', fontsize = 60)
# plt.yscale('log')
# plt.yticks(fontsize = 30)
# plt.xticks(ticks = np.concatenate((x, xb)), labels = np.concatenate((goodrunNo, badrunNo)), rotation = 90, fontsize = 10)
# plt.legend(fontsize = 20)
# # plt.ylim(32.2, 32.6) #zoom in to check that all runs are above the expected ratio
# # plt.savefig('./Plots/Timeratio.pdf')
# plt.show()


#########################################
##                                     ##
##    SECTION 3: DATA MERGING          ##
##                                     ##
#########################################



if not os.path.exists('./data/Merged/'):
        os.makedirs('./data/Merged/')



# data files
path = './data/goodRuns/csv/'
files = os.listdir(path)
files = [f for f in files]

# getrunNumber from string e.g., int('/data/hiion/scout_326676_000000.monitor.txt'.split('_')[1]
getRun = lambda x: int(x.split('.')[0])



# list to hold dataframes
dfList = []
runIndex= []

for file in files:
    filepath = path+file
    df = pd.read_csv(filepath)
    
  
    # add a column with the run number
    df['run'] = getRun(file) 
    dfList += [df]
    
    #make in index with the run numbers
    runIndex += [getRun(file)]




dfg = [] #data frame with only goodruns

for df in dfList:
    if df['run'][0] not in badrunNo : dfg.append(df)

# Make every df start at 1
for df in dfg:
    df.loc[:, 'orbit'] = df.loc[:, 'orbit'] - df.loc[0, 'orbit'] + 1

# Glue the next run orbit column to the end of the previous 

for j in range(1, len(dfg)):
    dfg[j].loc[:, 'orbit']+=dfg[j-1].loc[:, 'orbit'].tolist()[-1]

# Merge the runs
merged = pd.concat(dfg, ignore_index=True, sort=False)

merged.to_csv(path_or_buf="./data/Merged/merged.txt", index=False ) 



# 2-LEG MUONS

# ΔBx = 1

OS1, SS1 = pair(merged, 1)

print('len(OS1) / len(merged) = {:.4f}'.format(len(OS1)/len(merged)))

OS1.to_csv(path_or_buf="./data/Merged/twolegOS1.txt", index=False ) 
SS1.to_csv(path_or_buf="./data/Merged/twolegSS1.txt", index=False ) 


# ΔBx = 2

OS2, SS2 = pair(merged, 2)

print('len(OS2) / len(merged) = {:.4f}'.format(len(OS2)/len(merged)))

OS2.to_csv(path_or_buf="./data/Merged/twolegOS2.txt", index=False ) 
SS2.to_csv(path_or_buf="./data/Merged/twolegSS2.txt", index=False ) 

# ΔBx = 3

OS3, SS3 = pair(merged, 3)

print('len(OS3) / len(merged) = {:.4f}'.format(len(OS3)/len(merged)))

OS3.to_csv(path_or_buf="./data/Merged/twolegOS3.txt", index=False ) 
SS3.to_csv(path_or_buf="./data/Merged/twolegSS3.txt", index=False ) 