import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

file1 = open('data.txt', 'r')
Lines = file1.readlines()
pointsx =[]
pointsy = []
pointsz = []
for line in Lines:
    t = line.split(" ")
    t[2] = t[2][:-1]
    print(t)
    pointsx.append(float(t[0]))
    pointsy.append(float(t[1]))
    pointsz.append(float(t[2]))
ax.set_zlabel('Gas Density')
ax.set_xlabel('Height/Distance From Container')
ax.set_ylabel('Time')

ax.plot_trisurf(pointsx,  pointsz,pointsy, linewidth=0.1,cmap="gnuplot")
#ax.scatter(pointsx,pointsz,pointsy,)
plt.show()
