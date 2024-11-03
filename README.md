
<h2>Dependencies</h2>

To build and run the program, you need to have GStreamer and CMake installed on your machine.

<h2>Compiling and running </h2>

For compiling, I attached a CMakeLists file that will build the project. Compile the project using the following commands:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
~~~~~

This will create a GstSaverApp.exe inside the build/Debug folder on windows or inside build folder on linux. Then run it with the required arguments:
~~~
$ ./GstSaverApp.exe <pattern> <bitrate> <frames_per_file>
~~~

-----------------

<b>*) In my solution I used filesink instead of multifilesink because the mp4 files it produced wouldn't run for me. I think because of that, and bad EOS handling on my side, changing the location property didn't work for me - the pipeline wouldn't restart after pausing it and changing the property. So instead, I detached the sink element and re-attached a new one every rotation with the new file location. Unfortunately this is the only way I got this to work for me.. </b>
