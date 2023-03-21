# deform~

DSP waveshaper which uses random splines to generate its transfer function. A Max/MSP external object programmed in C++.


* The fully functional Max/MSP external is located at `/externals/deform~.mxe64`.
  * Note: changes to [min-devkit](https://github.com/Cycling74/min-devkit/tree/00fbf24c39169280f237c608892e402fcd1f53d9) made for Apple Silicon mean that this external will need to be rebuilt for Max 8.2 onwards.
* The associated C++ code is located at `/source/projects/deform_tilde/deform_tilde.cpp`.

## About

[deform~] generates a random spline $f_n: [-1,1] \rightarrow [-1,1]$, then a sequence of splines varying continuously from $f_n$ to $f_1(x) = x$. Combining these functions with an `Intensity` parameter $i\in[1,\dots,n]$, we obtain

$$\begin{aligned}f: [-1,1] \times \lbrace 1, \dots, n\rbrace &\rightarrow [-1,1]\\
f(x,i) &= f_i(x)\end{aligned}$$

This yields a waveshaper function whose output can be smoothly varied using the parameter. See below for sample output from an input of a sine wave, a sawtooth wave, and a triangle wave respectively.

![Sample output](https://user-images.githubusercontent.com/11036537/157645129-86b42829-dd46-475b-b6e4-d68513e04169.jpg)

https://user-images.githubusercontent.com/11036537/157646723-583e20cb-5d01-4b1c-b530-0e964a5b32f0.mp4

## Build

Make sure to initialise all the submodules after cloning this repo:

```
git submodule update --init --recursive
```

#### build on macOS

1. Have Xcode installed then run:
	```
	mkdir build
	cd build
	cmake -G Xcode ..
	```
2. Open up either `build/Min-Externals.xcodeproj` or the .xcodeproj for a specific external (e.g. `build/source/projects/lib.deform_tilde/lib.deform_tilde.xcodeproj`) in Xcode.
3. Set the build target to be 'Any Mac (Intel, Apple Silicon)'.
4. Press build. The compiled externals can be found in `externals/`.

#### build on Windows

1. Have Visual Studio installed then run `cmake --help` to get a list of the available generators. Pick the one that corresponds to the Visual Studio version installed e.g. `"Visual Studio 17 2022"`, then run:
	```
	mkdir build
	cd build
	cmake -G "Visual Studio 17 2022" ..
	```
2. Open up the Visual Studo project file, `deform_tilde.sln`, in `build/source/projects/deform_tilde/`.
3. In the menu bar, click 'Build > Build Solution'. The compiled object, `deform.maxpat`, can be found in `externals/`.
