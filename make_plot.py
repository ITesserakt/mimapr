import matplotlib.pyplot as plot
from matplotlib import cm, ticker
import matplotlib.animation as anim
from sys import stdin, argv
import numpy as np
from tqdm import tqdm

N = 1000

last = argv[1] == "-l"

matrix = np.array([list(map(float, line.split(' '))) for line in stdin]).reshape((N, 101, 81, 4))

fig, ax = plot.subplots()

if last:
    cs = ax.contourf(matrix[N - 1, :, :, 1], matrix[N - 1, :, :, 2], matrix[N - 1, :, :, 3], cmap=cm.plasma)
    fig.colorbar(cs)
    plot.show()
    exit(0)

pbar = tqdm(total=N)
cs = ax.contourf(matrix[0, :, :, 1], matrix[0, :, :, 2], matrix[0, :, :, 3], cmap=cm.plasma)

def update(i):
    global cs
    global pbar
    pbar.update(1)
    cs = ax.contourf(matrix[i, :, :, 1], matrix[i, :, :, 2], matrix[i, :, :, 3], cmap=cm.plasma)
    return cs


an = anim.FuncAnimation(fig, update, frames=N, interval=24)
fig.colorbar(cs)
an.save(filename="ani.gif", writer="pillow")

plot.show()
