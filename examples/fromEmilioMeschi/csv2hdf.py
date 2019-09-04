# original author: Emilio Meschi (2019)
import numpy as np
import pandas as pd
skiprows=0
numrows=100000
count = 0
columns = ['phi','phie','eta','etae','pt','qual','charge','tpt','tphi','teta']

store=pd.HDFStore('charm_full.hd5',complevel=9)

while True:
    data=np.genfromtxt("charm_full.txt",delimiter=",",skip_header=skiprows,max_rows=numrows,names=columns)
    skiprows+=numrows
    count+=1
    df = pd.DataFrame(data=data,columns=columns)
    store.append('scout', df)
    print count,count*numrows
    if data.shape[0] == 0:
        break
