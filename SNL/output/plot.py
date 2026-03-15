import numpy as np
import matplotlib.pyplot as plt

points = np.loadtxt("Points.dat")
data = np.loadtxt("Plot.dat")

print(data)

plt.plot(data[:, 0], data[:, 1])
plt.plot(points[:, 0], points[:, 1])

plt.show()