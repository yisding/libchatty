# LangChain

## Results

```
	Command being timed: "./loop.sh"
	User time (seconds): 6.24
	System time (seconds): 1.42
	Percent of CPU this job got: 13%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:58.75
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 101056
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 356
	Minor (reclaiming a frame) page faults: 165103
	Voluntary context switches: 911
	Involuntary context switches: 15600
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 130
	Socket messages received: 261
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### LangChain 0.2.14

```
	Command being timed: "./loop.sh"
	User time (seconds): 7.21
	System time (seconds): 1.86
	Percent of CPU this job got: 20%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:44.81
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 103360
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 195
	Minor (reclaiming a frame) page faults: 163938
	Voluntary context switches: 421
	Involuntary context switches: 23440
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 130
	Socket messages received: 265
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

Literally 40x the CPU time and 14x the memory usage of libchatty.
