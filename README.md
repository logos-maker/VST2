# VST2 API example with editor, for Windows and Linux
This substitutes the smallest amout of code, for making a cross platform VST2 audio plugin with a graphical editor.

, with cross platform code that subrogates the VST2 licence. That makes it possible for anyone to make VST2 ABI compatible audio plugins, that will be fully usefull in numerus music programs.

#Subrogating the VST2 licence - Making VST2 developent availabe for anyone
To circumvent this issue, this codebase only makes use of parts of the VST2 ABI.

We could have kept the original API for VST2 plugins, as it's proven in court that you can't protect a API from common usage according to the the U.S. Supreme Court’s April 5 ruling in Google LLC v. Oracle America Inc. in 2021. And it's certinly the case that you can't protect an ABI with copyright within all reasonable doubt. Therefore this code can be used without any concerns of any third party licences.

This makes it possible for anyone to make an audio plugin in C code with an graphical editor, with the least amount of convolution. This makes compiled binaries usable in the most amoun't of music programs, with the least amount of code and dependencies.

I have kept the code size as low as possible, and made it compilable with Linux and Windows without any third party libraries.


