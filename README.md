# NNProfiler
Gstreamer real-time profiler plugin for nnstreamer

## Open Source License
- GstShark
https://github.com/RidgeRun/gst-shark

- For further information visit
https://developer.ridgerun.com/wiki/index.php?title=GstShark

## How To Install
```
$ ./autogen_ubuntux64.sh //on ubuntu x64, for other environment visit wiki
$ ./make
$ ./sudo make install
```


## How To Use NNProfiler
Set environment variable GST_DEBUG, GST_TRACERS to tracers to use
```
$ GST_DEBUG="GST_TRACER:7" GST_TRACERS="cpuusage;proctime;interlatency;framerate"\
     gst-launch-1.0 videotestsrc ! videorate max-rate=15 ! fakesink
```


