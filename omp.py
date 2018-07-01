import numpy as np
import matplotlib.pyplot as pp

thread,t=np.loadtxt("omp.dat",unpack=True)

num=[0]*20
times=[0]*20

for i in range(len(thread)):
    index = int(thread[i])-1
    num[index]+=1
    times[index]+=t[i]

print num

for i in range(len(times)):
    times[i]=times[i]/float(num[i])
print times

nu=[]
for i in range(len(times)):
    nu.append(i+1)

pp.scatter(nu,times)
pp.xlabel("Number of threads")
pp.xlim(0,21)
pp.ylim(bottom=0)
pp.ylabel("Time (seconds)")
pp.title("OpenMP runs")
pp.savefig("omp.pdf")
