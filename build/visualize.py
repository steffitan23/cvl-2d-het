import numpy as np
import os
import imageio
from IPython.display import Video

for file in os.listdir('u_arr'):
    if file.endswith(".npy"):
        arr = np.load('u_arr/' + file)
        imageio.mimwrite(f'{file}.mp4', arr, fps=10)
        