import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm

array = np.loadtxt("output.dat",unpack=True)
print "done loading, now imaging"

im = plt.imshow(array,cmap='hot_r',origin='lower',extent=(-2,.5,-2,2),aspect='equal')
print "done imaging"

plt.colorbar(im,label="Number of iterations to divergence")
print "colorbar done"

plt.xlabel("Real axis")
plt.ylabel("Imaginary axis")

plt.savefig("mandelbrot.png",dpi=600)
