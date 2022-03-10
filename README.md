# deform~

DSP waveshaper which uses random splines to generate its transfer function. A Max/MSP external object programmed in C++.

## About

[deform~] generates a random spline `fn: [-1,1] -> [-1,1]`, then a sequence of splines varying continuously from `fn` to `f1(x) = x`. Combining these functions with an `Intensity` parameter ranging from `1` to `n`, we obtain

```
f: [-1,1] x {1, ..., n} -> [-1,1]  
f(x,i) = f_i(x)
```

This yields a waveshaper function whose output can be smoothly varied using the parameter. See below for sample output from an input of a sine wave, a sawtooth wave, and a triangle wave respectively.

![Sample output](/testing/example.jpg)

## To-do

* Add Infinite Linear Oversampling to resolve aliasing issues. Suggested approach to this in testing/deform_float.cpp.

* Add attributes to [lib.deform\~] so number of polynomial pieces in spline, and random seed, can be set in the object.

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

1. Have Visual Studio installed then run `cmake --help` to get a list of the available generators. Pick the one that corresponds to the Visual Studio version installed e.g. `"Visual Studio 16 2019"`, then run:
	```
	mkdir build
	cd build
	cmake -G "Visual Studio 16 2019" ..
	```
2. Open up one of the Visual Studo project files, either the main one in `build/` or one for a specific external (e.g. in `build/source/projects/lib.deform_tilde/`).
3. In the menu bar, click something along the lines of 'Build > Run Build'. The compiled externals can be found in `externals/`.
