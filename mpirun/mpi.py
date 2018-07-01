import numpy as np
import matplotlib.pyplot as pp

p,t=np.loadtxt("dat.dat",unpack=True)

num=[0]*20
times=[0]*20

for i in range(len(p)):
    index = int(p[i])/2-1
    num[index]+=1
    times[index]+=t[i]

print num

for i in range(len(times)):
    times[i]=times[i]/float(num[i])
print times

nu=[]
for i in range(len(times)):
    nu.append(i*2+2)

pp.scatter(nu,times)
pp.xlabel("Number of processors")
pp.xlim(0,41)
pp.ylim(bottom=0)
pp.ylabel("Time (seconds)")
pp.title("MPI runs")
pp.savefig("mpi.png")
