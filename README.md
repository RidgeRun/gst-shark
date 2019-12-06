# NNProfiler
Gstreamer real-time profiler plugin for nnstreamer

## Open Source License
- GstShark
https://github.com/RidgeRun/gst-shark

- For further information visit
https://developer.ridgerun.com/wiki/index.php?title=GstShark

## How To Install
- On Ubuntu x64
```console
$ sudo apt install gtk-doc-tools libgraphviz-dev libncurses5-dev libncursesw5-dev
$ ./autogen_ubuntux64.sh
$ make
$ sudo make install
```
For other environment, visit GstShark wiki


## How To Use NNProfiler
Set environment variable GST_DEBUG, GST_TRACERS to tracers to use and NNPROFILER_ENABLED to TRUE
```console
$ NNPROFILER_ENABLED=TRUE GST_DEBUG="GST_TRACER:7" GST_TRACERS="cpuusage;proctime;interlatency;framerate"\
     gst-launch-1.0 videotestsrc ! videorate max-rate=15 ! fakesink
```


