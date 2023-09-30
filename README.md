# VST2 API example with editor, for Windows and Linux
This is the smallest amout of code, for making a cross platform VST2 audio plugin with a graphical editor yet.

This code was written on Linux, and cross compiles to Windows. And it subrogates the VST2 licence. That makes it possible for anyone to make VST2 ABI compatible audio plugins, that will be fully usefull in numerus music programs on Windows developing an a Linux machine.

#2 Subrogating the VST2 licence - Making VST2 developent availabe for anyone
To circumvent this issue, this codebase only makes use of parts of the VST2 ABI.

I could have kept the original API for VST2 plugins, but a specific company would complain and make it go away, when I'm making this public if I took that route.

It's proven in court that you can't protect a API from common usage according to the the U.S. Supreme Court’s April 5 ruling in Google LLC v. Oracle America Inc. in 2021. And it's certinly the case that you can't protect an ABI with copyright within all reasonable doubt. Therefore this code can be used without any concerns of any third party licences, as it only make use of the ABI, and any resemblence of the original API could be well motivated as it's not even prohibeted to make use of the original API without any following of licensenses. But I have taken a other route, that will make it possible to make a new standard futher along the way.

This example, makes it possible for anyone to make an audio plugin in C code with an graphical editor, with the least amount of convolution. This makes compiled binaries usable in the most amoun't of music programs, with the least amount of code and dependencies. This makes it easy to support anything that has VST2 plugin capabillity. It currenty only supports 64bit platforms. But I'm looking for a way to support 32 bit systems also with only one line of code, as I think 2 or 3 lines for that is to much.

I have kept the code size as low as possible, and made it compilable with Linux and Windows without any third party libraries. And with give the most copatible code to the most systems. And it's also ensured that compiled windows plugins, can run on a Linux machine with Wine without any tweakeing.






