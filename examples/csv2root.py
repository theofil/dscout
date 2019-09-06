import pandas
from root_pandas import readwrite
filename = 'scout_326790_000000_trunc.csv'
df = pandas.read_csv(filename)
df.to_root(filename[:-3]+'root','events')
df.to_hdf(filename[:-3]+'h5', key='df', mode='w')
df.to_hdf(filename[:-3]+'zlib_9.'+'h5', key='df', mode='w', complib ='zlib', complevel=9)
df.to_hdf(filename[:-3]+'zlib_1.'+'h5', key='df', mode='w', complib ='zlib', complevel=1)
df.to_hdf(filename[:-3]+'bzip2_9.'+'h5', key='df', mode='w', complib ='bzip2', complevel=9)
df.to_hdf(filename[:-3]+'bzip2_1.'+'h5', key='df', mode='w', complib ='bzip2', complevel=1)

