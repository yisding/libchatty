# libchatty is a ChatGPT Wrapper [for Ricers](https://www.shlomifish.org/humour/by-others/funroll-loops/Gentoo-is-Rice.html)

Is your ChatGPT wrapper too slow? Is it causing you trouble in VC pitches? API changes frustrating your dev team? Wow, do I have the solution for you. libchatty is the world's first high performance ChatGPT wrapper written 100% in handcrafted, artisinal, C.

libchatty is 281x faster than [LlamaIndex](llamaindex/README.md) and uses 12x less memory than [Langchain](langchain/README.md).

## FAQ

### OMG this is so amazing what inspired you to make libchatty?

When you've been using LLMs as long as I have (since 2023-01-21) you accumulate these people called friends, and sometimes these friends, like people from Vespa, Qdrant, and Weaviate, come up with [some](https://x.com/jobergum/status/1809157587612336402) [really](https://x.com/philipvollet/status/1809498065998393650) [good](https://x.com/generall931/status/1809499192982725029) [ideas](https://x.com/jobergum/status/1810408969468276868) . And then you can steal these ideas and start a project, and maybe even a company.

### Why do you use libcurl? Why didn't you write your own HTTP library?

First off, if you haven't heard of LD_PRELOAD/LD_LIBRARY_PATH I can't be wasting my precious milliseconds talking to you. Secondly, GitHub Copilot's C support is quite lacking, and I don't have access to Devin.

### Do you take contributions?

No, unless you're committed to ricing, in which case, yes.

### Well, at least are there Python or JS bindings?

Yup, [Python](https://github.com/openai/openai-python) and [JS](https://github.com/openai/openai-node). Please report any bugs in their respective repos and not here.

### Does libchatty support hybrid search?

Your mom. (what?) Your mom is who supports hybrid search. This is a light weight blazing fast ChatGPT wrapper for AI Engineering chads, not a do everything for you RAG 15.0 Pro Max library. If you want one of those, you should BM [these folks](https://x.com/LoganMarkewich/status/1810122047235961258).

## Results

```
	Command being timed: "./loop.sh"
	User time (seconds): 0.06
	System time (seconds): 0.08
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:34.94
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 6192
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 80
	Minor (reclaiming a frame) page faults: 6209
	Voluntary context switches: 158
	Involuntary context switches: 608
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 396
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### I can't reproduce your results.

That's because you don't own my laptop. [DM me your results on Twitter.](https://x.com/yi_ding)