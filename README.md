# libchatty is a ChatGPT Wrapper [for Ricers](https://www.shlomifish.org/humour/by-others/funroll-loops/Gentoo-is-Rice.html)

Is your ChatGPT wrapper too slow? Is it causing you trouble in VC pitches? API changes frustrating your dev team? Wow, do I have the solution for you. libchatty is the world's first high performance ChatGPT wrapper written 100% in handcrafted, artisinal, C.

libchatty is 212x faster than [LlamaIndex](llamaindex/README.md), 40x faster than [LangChain](langchain/README.md) and is even 20x faster than [LITS 🔥](lits/README.md).

## Supported Providers

libchatty supports multiple AI providers through environment variables:

| Provider | Base URL | Environment Variable |
|----------|----------|---------------------|
| OpenAI | `https://api.openai.com/v1` | `OPENAI_API_KEY` |
| Groq | `https://api.groq.com/openai/v1` | `GROQ_API_KEY` |
| Fireworks AI | `https://api.fireworks.ai/inference/v1` | `FIREWORKS_API_KEY` |
| Mistral AI | `https://api.mistral.ai/v1` | `MISTRAL_API_KEY` |
| Hyperbolic | `https://api.hyperbolic.xyz/v1` | `HYPERBOLIC_API_KEY` |
| DeepSeek | `https://api.deepseek.com/v1` | `DEEPSEEK_API_KEY` |
| Llama API (Meta) | `https://api.llama.com/v1` | `LLAMA_API_KEY` |

Set the `OPENAI_API_BASE` environment variable to use a different provider:

```bash
# Use Groq
OPENAI_API_BASE="https://api.groq.com/openai/v1" ./build/chatty llama-3.1-70b-versatile

# Use Llama API (Meta)
OPENAI_API_BASE="https://api.llama.com/v1" ./build/chatty Llama-4-Maverick-17B-128E-Instruct-FP8

# Use OpenAI (default)
./build/chatty gpt-4o
```

## FAQ

### OMG this is so amazing what inspired you to make libchatty?

When you've been using LLMs as long as I have (since 2023-01-21) you accumulate these people called friends, and sometimes these friends, like people from Vespa, Qdrant, and Weaviate, come up with [some](https://x.com/jobergum/status/1809157587612336402) [really](https://x.com/philipvollet/status/1809498065998393650) [good](https://x.com/generall931/status/1809499192982725029) [ideas](https://x.com/jobergum/status/1810408969468276868). And then you can steal these ideas and start a project, and maybe even a company.

### Why do you use libcurl? Why didn't you write your own HTTP library?

First off, if you haven't heard of LD_PRELOAD/LD_LIBRARY_PATH I can't be wasting my precious milliseconds talking to you. Secondly, GitHub Copilot's C support is quite lacking, and I don't have access to Devin.

### Do you take contributions?

No, unless you're committed to ricing, in which case, yes.

### Well, at least are there Python or JS bindings?

Yup, [Python](https://github.com/openai/openai-python) and [JS](https://github.com/openai/openai-node). Please report any bugs in their respective repos and not here.

### Does libchatty support hybrid search?

Your mom. (what?) Your mom is who supports hybrid search. This is a light weight blazing fast ChatGPT wrapper for AI Engineering chads, not a do everything for you RAG 15.0 Pro Max library. If you want one of those, you should BM [these folks](https://x.com/LoganMarkewich/status/1810122047235961258).

## Benchmark Results

### Llama 3.1 8B (Groq):
```
	Command being timed: "./groq.sh"
	User time (seconds): 0.07
	System time (seconds): 0.09
	Percent of CPU this job got: 2%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:08.42
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 5824
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 41
	Minor (reclaiming a frame) page faults: 6425
	Voluntary context switches: 104
	Involuntary context switches: 623
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 388
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### Llama 3.1 70B (Groq)
```
	Command being timed: "./groq.sh"
	User time (seconds): 0.03
	System time (seconds): 0.04
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:17.98
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 5920
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 21
	Minor (reclaiming a frame) page faults: 6433
	Voluntary context switches: 122
	Involuntary context switches: 562
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 410
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### Llama 3.1 70B (Fireworks)
```
	Command being timed: "./fireworks.sh"
	User time (seconds): 0.04
	System time (seconds): 0.05
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:29.64
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 6016
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 52
	Minor (reclaiming a frame) page faults: 6311
	Voluntary context switches: 139
	Involuntary context switches: 604
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 390
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### Llama 3.1 405B (Fireworks)
```
	Command being timed: "./fireworks.sh"
	User time (seconds): 0.08
	System time (seconds): 0.10
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 1:40.66
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 6176
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 21
	Minor (reclaiming a frame) page faults: 6331
	Voluntary context switches: 194
	Involuntary context switches: 636
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 390
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### Mistral Large 2 (Le Platforme)
```
	Command being timed: "./mistral.sh"
	User time (seconds): 0.07
	System time (seconds): 0.10
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 1:03.76
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 6512
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 34
	Minor (reclaiming a frame) page faults: 6443
	Voluntary context switches: 166
	Involuntary context switches: 854
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 60
	Socket messages received: 388
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### GPT-4o:
```
	Command being timed: "./loop.sh"
	User time (seconds): 0.08
	System time (seconds): 0.11
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:51.48
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 7232
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 44
	Minor (reclaiming a frame) page faults: 7093
	Voluntary context switches: 244
	Involuntary context switches: 881
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 160
	Socket messages received: 278
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### GPT-4.5-preview (2025/2/27)
```
	Command being timed: "./loop.sh gpt-4.5-preview"
	User time (seconds): 0.37
	System time (seconds): 0.14
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 6:51.16
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 14784
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 471
	Minor (reclaiming a frame) page faults: 11630
	Voluntary context switches: 570
	Involuntary context switches: 1200
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 161
	Socket messages received: 300
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### GPT-4.5-preview (2025/2/28)
```
	Command being timed: "./loop.sh gpt-4.5-preview"
	User time (seconds): 0.38
	System time (seconds): 0.18
	Percent of CPU this job got: 0%
	Elapsed (wall clock) time (h:mm:ss or m:ss): 5:09.60
	Average shared text size (kbytes): 0
	Average unshared data size (kbytes): 0
	Average stack size (kbytes): 0
	Average total size (kbytes): 0
	Maximum resident set size (kbytes): 14720
	Average resident set size (kbytes): 0
	Major (requiring I/O) page faults: 800
	Minor (reclaiming a frame) page faults: 12758
	Voluntary context switches: 551
	Involuntary context switches: 1965
	Swaps: 0
	File system inputs: 0
	File system outputs: 0
	Socket messages sent: 167
	Socket messages received: 303
	Signals delivered: 10
	Page size (bytes): 16384
	Exit status: 0
```

### I can't reproduce your results.

That's because you don't own my laptop. [DM me your results on Twitter.](https://x.com/yi_ding)