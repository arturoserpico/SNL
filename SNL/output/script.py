import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection

positions = np.loadtxt("Positions.dat")
edges = np.loadtxt("Edges.dat")
edges = edges.astype(int)
faces = np.loadtxt("Faces.dat")
faces = faces.astype(int)

segments = np.array([
    [positions[e[0], :], positions[e[1], :]]
    for e in edges
])

print(segments)

edgesCenters = np.zeros((edges.shape[0], 2))

for i in range(0, edges.shape[0]):
    edgesCenters[i, 0] = (segments[i, 0, 0] + segments[i, 1, 0]) / 2
    edgesCenters[i, 1] = (segments[i, 0, 1] + segments[i, 1, 1]) / 2

facesCenter = np.zeros((faces.shape[0], 2))
facesSegment = []

for i in range(0, faces.shape[0]):
    sumV = np.zeros(2)

    for j in faces[i, :]:
        p = edgesCenters[j, :]
        sumV += p
    
    facesCenter[i, :] = sumV / faces[i, :].size

    for k in faces[i, :]:
        facesSegment.append([facesCenter[i, :], edgesCenters[k, :]])

fig, ax = plt.subplots()

edgesCollection = LineCollection(segments, colors='blue', linewidths=1.0)
centerCollection = LineCollection(facesSegment, colors='red', linewidths=0.5)
ax.add_collection(edgesCollection)
ax.add_collection(centerCollection)

# Plot nodes
ax.scatter(positions[:, 0], positions[:, 1], color='black', s=10)
ax.scatter(edgesCenters[:, 0], edgesCenters[:, 1], color='green', s=10)
ax.scatter(facesCenter[:, 0], facesCenter[:, 1], color="red", s=10)

ax.set_aspect('equal')
ax.autoscale()
ax.set_title("FEM Mesh")

plt.show()