
<h2>Dependencies</h2>

To build and run the program, you need to have GStreamer and CMake installed on your machine.

<h2>Compiling</h2>

I attached a CMakeLists file that will build the project. To build the project you can use the build_project.sh script for Linux or build_project.ps1 script for Windows, those will build the project for you. 
Alternatively, you can build the project yourself using the following commands:

~~~~~
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
~~~~~

<h2>Running the program</h2>
Building the project will create a GstSaverApp(.exe) inside the build/Debug folder on windows or inside build folder on linux. Then cd into the directory and run it with the required command line arguments:

~~~
$ ./GstSaverApp.exe <pattern> <bitrate> <frames_per_file>
~~~

-----------------

<b>*) In my solution I used filesink instead of multifilesink because the mp4 files it produced wouldn't run for me. I think because of that, and bad EOS handling on my side, changing the location property didn't work for me - the pipeline wouldn't restart after pausing it and changing the property. So instead, I detached the sink element and re-attached a new one every rotation with the new file location. Unfortunately this is the only way I got this to work for me.. </b>
