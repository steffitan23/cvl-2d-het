from pathlib import Path
from tracemalloc import start
from typing import Union
import re

import numpy as np
from tqdm import tqdm

import matplotlib.pyplot as plt
from skimage import draw
from scipy.ndimage import gaussian_filter

import os

import random

# Create array with specified base value of a,coords of the heterogeneity, number of heterogeneities, local a value of heterogeneity

def create_hetero_arr(arr, x, y, rad, het, sigma):
    rr, cc = draw.disk((x, y), rad, shape=arr.shape)
    arr[rr, cc] = het
    blurred = gaussian_filter(arr, sigma=sigma)
    return blurred

def randomize_hetero(num):
    arr = np.zeros((200, 200))
    base = np.random.choice(np.arange(0.07, 0.1, 0.01))
    arr[:] = base
    res = arr
    for i in range(num):
        x = random.randint(0, 200)
        y = random.randint(0, 200)
        rad = random.randint(1, 40)
        het = np.random.choice(np.arange(0.05, 1.01, 0.01))
        sigma = random.randint(1, 10)
        print(f"Creating heterogeneity (no. {i}) at ({x}, {y}) with a = {het} with gaussian filter standard dev = {sigma} and base = {base}:)")
        res = create_hetero_arr(res, x, y, rad, het, sigma)
    return res

def save_arr(save_arr, index):
    plt.imshow(save_arr)
    plt.title("Heterogeneities plot")
    plt.show()
    np.save("local_parameter_arrays/a" + str(index) + ".npy", save_arr)

# Run simulation TRIAL times
def run_sim(trials):
    for i in range(trials):
        het_num = random.randint(1, 6)
        gen_arr = randomize_hetero(het_num)
        save_arr(gen_arr, i)
        # Overwrite a.npy in build
        np.save("a.npy", gen_arr)
        # Run simulation
        cmd = "./em2d_cluster --load ../sim_conf.ini"
        os.system(cmd)
        os.rename("Messdaten/px100py100_fib0deg_k8_D1_a0.05_e00.01_mu10.07_mu20.3_Gs0.1", f"new_data/data{i}")
        save_frames(i)

def tung_load_dir(dir_path: Union[Path, str]):
    def tung_load_bin(binary_file: Path):
        dt = np.dtype([('u', np.float64),
                       ('v', np.float64),
                       ('Ta', np.float64),
                       ('curr_pos', np.float64, (2,)),
                       ('prev_pos', np.float64, (2,)),
                       ('curr_vel', np.float64, (2,))])

        with open(binary_file, 'rb') as f:
            b = f.read(8)
            area_size, run_time = np.frombuffer(b, dtype=np.int32)
            a = np.fromfile(f, dtype=dt).reshape((area_size, area_size))

        return a['u'], a['curr_pos']

    def _natural_sort_path_key(path: Path, _nsre=re.compile('([0-9]+)')):
        return [int(text) if text.isdigit() else text.lower() for text in _nsre.split(path.name)]

    def iter_dir_natural_sorted(dir_path: Path, glob_str: str):
        return sorted(dir_path.glob(glob_str), key=_natural_sort_path_key)

    if isinstance(dir_path, str):
        dir_path = Path(dir_path)
    assert dir_path.is_dir()
    u_arr = []
    pos_arr = []
    for p in tqdm(iter_dir_natural_sorted(dir_path, "*.bin")):
        u, pos = tung_load_bin(p)
        u_arr.append(u)
        pos_arr.append(pos)

    return np.array(u_arr), np.array(pos_arr)

# Save 10 frames from start, middle, end section of the simulated array after first 100 frames
def save_frames(index):
    u_arr, positions = tung_load_dir("./new_data/data" + str(index))
    # load_data(u_arr)
    u_arr = u_arr[101:401]
    saved = []
    for i in range(3):
        saved.append(check_usable(u_arr))
        u_arr= u_arr[101:]
    for portion_frames in range(len(saved)):
        if np.all(saved[portion_frames]) != None:
            np.save("u_arr/data" + str(index) + "_" + str(portion_frames), saved[portion_frames])
            
def check_usable(save_arr):
    for j in np.arange(0, 101, 10):
            bool_arr = save_arr[j] > 0.6
            if np.count_nonzero(bool_arr) < 140:
                print("unusable")
                break
            return save_arr[0:101:10]

# def load_data():
#     import imageio
#     from IPython.display import Video
#     imageio.mimwrite('take1.mp4', np.load('u_arr/data0_1.npy'), fps=30)
run_sim(5)
# load_data()
# a values should be in between 0.05 and 0.1 (steps of 0.01)
# k values should be 7, 8 or 9