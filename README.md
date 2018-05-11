Magnum VR UI
============

*Magnum::Ui + Leap Motion + Oculus Rift VR*

![magnum-vr-ui.gif](img/magnum-vr-ui.gif)

Mix of [Magnum Ui Gallery](http://magnum.graphics/showcase/magnum-ui-gallery/),
[Magnum Oculus VR example](http://doc.magnum.graphics/magnum/examples-ovr.html) and
[Magnum Leap Motion example](http://doc.magnum.graphics/magnum/examples-leapmotion.html),
showing how to fusion them all together.

As the intersection of platforms supported by both Leap Motion and Oculus is Windows, only
Windows is supported.

# Building

You may be better off just downloading a build from [releases](https://github.com/Squareys/magnum-vr-ui/releases) instead.
But if you insist:

The project relies on [CMake](https://cmake.org/). You can use [vcpkg](https://github.com/Microsoft/vcpkg) or
[build Corrade, Magnum, MagnumIntegration and MagnumExtras yourself](http://doc.magnum.graphics/magnum/building.html).
After that, building this project should be a matter of:

~~~
mkdir build && cd build
cmake ..
cmake --build . --target install
~~~

Potentially with additional parameters like `CMAKE_INSTALL_PREFIX` or
`CMAKE_TOOLCHAIN_FILE` depending on your specific setup.

# Licence

The code of this project is licensed under the MIT/Expat license:

~~~
Copyright © 2018 Jonathan Hale <squareys@googlemail.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
~~~
