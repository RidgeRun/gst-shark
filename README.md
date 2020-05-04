# Live Profiler for Gstreamer
Real-time profiler plugin for Gstreamer

## Open Source License
- GstShark

https://github.com/RidgeRun/gst-shark

- For further information, visit GstShark wiki

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


## How To Use Live Profiler
Set environment variable as below.
- GST_DEBUG = "GST_TRACER:7"
- GST_TRACERS = "live;" + (tracers to use)
```console
$ GST_DEBUG="GST_TRACER:7" GST_TRACERS="live"\
     gst-launch-1.0 videotestsrc ! videorate max-rate=15 ! fakesink
```


