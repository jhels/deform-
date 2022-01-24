# Min Devkit Externals

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
2. Open up either `build/Min-Externals.xcodeproj` or the .xcodeproj for a specific external (e.g. `build/source/lib.deform_tilde/lib.deform_tilde.xcodeproj`) in Xcode.
3. Set the build target to be 'Any Mac (Intel, Apple Silicon)'.
4. Press build. The compiled externals can be found in `externals/`.

#### build on Windows

1. Have Visual Studio installed then run `cmake --help` to get a list of the available generators. Pick the one that corresponds to the Visual Studio version installed e.g. `"Visual Studio 16 2019"`, then run:
	```
	mkdir build
	cd build
	cmake -G "Visual Studio 16 2019" ..
	```
2. Open up one of the Visual Studo project files, either the main one in `build/` or one for a specific external (e.g. in `build/source/lib.deform_tilde/`).
3. In the menu bar, click something along the lines of 'Build > Run Build'. The compiled externals can be found in `externals/`.
