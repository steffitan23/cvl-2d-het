from pathlib import Path
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
    arr[:] = np.random.choice(np.arange(0.05, 1.01, 0.01))
    res = arr
    for i in range(num):
        x = random.randint(0, 200)
        y = random.randint(0, 200)
        rad = random.randint(1, 40)
        het = np.random.choice(np.arange(0.05, 1.01, 0.01))
        sigma = random.randint(1, 10)
        print(f"Creating heterogeneity (no. {i}) at ({x}, {y}) with a = {het} with gaussian filter standard dev = {sigma} :)")
        res = create_hetero_arr(res, x, y, rad, het, sigma)
    return res

def save_arr(save_arr, index):
    plt.imshow(save_arr)
    plt.title("Heterogeneities plot")
    plt.show(block=False)
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

run_sim(3)
# Save 10 frames from a random section of the simulated array

# a values should be in between 0.05 and 0.1 (steps of 0.01)
# k values should be 7, 8 or 9