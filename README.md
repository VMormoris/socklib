
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
<br>

```socklib``` is an open source C++ library that aims to provide a Platform agnostic Python-Like Socket object.

## Requirements

To be able to use ```socklib```, you will need a modern compiler that supports C++17.<br>
The following requirements are also necessary:
* ```premake5``` with the ability to run it from the command line.
* ```Visual Studio``` a version that supports C++17 (__only on Windows__)
* ```make``` (__on Linux__)

_You can find more information on how to install & use ```premake5``` [here](https://premake.github.io/docs/Using-Premake)._

## Build & Testing
Start by cloning the repository with: ```git clone https://github.com/VMormoris/socklib```.<br>
Then depending on the platform you use you have to generate and build the project following those steps: 
* __On Windows:__<br>
  * ```premake5 vs20XX``` _(where XX the version of Visual Studio you have)_
  * Open the solution file that was generate in order to build the library and run the tests
* __On Linux:__<br>
  * ```premake5 gmake2```
  * ```make``` to build the library and the tests
  * ```./bin/Debug-linux/tests/tests``` to run the tests

## Using
In order to use the library into your own projects you will need:
* Add the ```include``` as an include directory on your project
* Link your project with the apropriate ```socklib.lib``` file _(Debug or Release)_

## Third Party Libraries
* [Catch2](https://github.com/catchorg/Catch2) is used as a unit testing framework.
* [premake5](https://github.com/premake/premake-core) is used for project generation.

## Contributing
* Fell free to contribute to any of the [open issues](https://github.com/VMormoris/socklib/issues) or open a new one describing what you want to do.
* For small scale improvements and fixes simply submit a GitHub pull request
