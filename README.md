A 2D electro-mechanical simulator. It can simulate isotropic and anisotropic wave propagation on a 2D domain. SAC is also included. It was developed in CodeLite, but with the help of Jan L. and CMake, the entire program can be build without CodeLite.

## Compilation

This project uses [CMake](https://cmake.org/) to generate cross-platform build configurations. Either use `cmake-gui` or the terminal from the project folder (where *CMakeList* is located):

```bash
# Create a subfolder (generally called build)
mkdir build
cd build

# Generate a project for the default platform
cmake ..

# Alternatively display the available platforms and generate the project for the platform of your choice
cmake --help    (optional)
cmake .. -G "Xcode"    (example)
cmake .. -G "Unix Makefiles"    (example)
cmake .. -G "Visual Studio 15 2017 Win64"    (example)

# Then use your IDE or use CMake to compile
cmake --build .
```

## How to use

After a successful build, two executables can be found in the build folder. The `em2d_cluster` is to be used on a cluster, since it does not have any visual dependencies included. It is so to say recommended for parallel calculation on the cluster. The `em2d_viewer` is instead for the desktop, since it requires freeglut for graphical rendering, which should be used for generating images or videos out of images.

There is a sim_conf.ini file, which should be loaded with
```
./em2d_viewer --load <path to sim_conf.ini>

./em2d_cluster --load <path to sim_conf.ini>

# a concrete example
./em2d_viewer --load ../sim_conf.ini
```
since it has basic configuration for further use already. Any modification can be done in this ini-file without need for recompiling. If you start the viewer, the program will present you with some views, like the fields u-value, v-value or the stretch-map.

To initialize a simulation, you can either use the key `d` for a stimulated "dot", `s` for a spirale or `c` for "chaos" or a predefined fibrillation, constisting of several spirals. After pressing any of these keys once, press `space` to actually start the simulation.

To take a picture, press `p`. A screengrab will be saved with the current date and time in the filename.

While the simulation runs, you can also just randomly draw with the mouse in the u-field, if you're bored.

Further configurable parameters can be taken from the `sim_conf.ini` file.