FreeSpace2 *S*ource *C*ode *P*roject
==
[![Coverity](https://img.shields.io/coverity/scan/870.svg)](https://scan.coverity.com/projects/870)

Headtracking Fork
-- 
This is a temporary **fork** of [fs2open](https://github.com/scp-fs2open/) that has the `facetracknoir` headtracker protocol added on Linux PC. That's afaik what OpenTrack uses too when `Output UDP` is selected. This is *not* build at all when `WIN32` is set (see `headtracking/headtracking.cpp`). I simply can't test this on Windows so YMMV. 

As for Linux check `headtracking/facetracknoir.cpp` for details. This raises the hardcoded UDP port `0.0.0.0:5550` when the game is started. You can confirm this e.g. with `ss -uln` where the port should become available.

You can launch OpenTrack any time. Set it's output to `OpenTrack UDP` and have it configured to send data to `120.0.0.1` and port `5550`. You may have to invert yaw/pitch. Within the game you **must be in** the **cockpit view** for this to have any effect.

![description](https://beko.famkos.net/wp-content/uploads/2026/02/fs2open-headtracking-linux-pc.jpg)

Good hunting o7

Building
--
Before you do anything, make sure you have updated your git submodules, either by running `git submodule update --init --recursive` or by cloning the repository with the `--recursive` flag.<br/>

The main instructions for building can be found at our github wiki, on the [Building](https://github.com/scp-fs2open/fs2open.github.com/wiki/Building) page.