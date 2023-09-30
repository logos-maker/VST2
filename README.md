# VST2 API example with editor, for Windows and Linux
This is the smallest amount of code, for making a cross platform VST2 audio plugin with a graphical editor yet.

This code was written on Linux, and cross compiles to Windows. And it subrogates the VST2 license. That makes it possible for anyone to make VST2 ABI compatible audio plugins, that will be fully useful in numerous music programs on Windows, developing an a Linux machine.

## Subrogating the VST2 licence - Making VST2 development availabe for anyone
To make this available and useful for anyone, this code-base only makes use of parts of the VST2 ABI.

I could have kept the original API for VST2 plugins, but a specific company would complain. To get rid of this type of complaints, I've settle to use a fully compatible route, that circumvent this issue, when making this public.

It's proven in court that you can't protect a API from common usage according to the the U.S. Supreme Court’s April 5 ruling in Google LLC v. Oracle America Inc. in 2021. And it's certainly the case that you can't protect an ABI with copyright within all reasonable doubt. Therefore this code can be used without any concerns of any third party licenses, as it only make use of the ABI, and any resemblance of the original API could be well motivated as it's not even prohibited to make use of the original API without any violation of any original licenses.

This example, makes it possible for anyone to make an audio plugin in C code with a graphical editor, with the least amount of convolution. This makes compiled binaries usable in the most amount of music programs, with the least amount of code and dependencies. This makes it easy to support anything that has VST2 plugin capability. It currently only supports 64-bit platforms. But I'm looking for a way to support 32-bit systems also with only one line of code, as I think 2 or 3 lines for that is too much.

I have kept the code size as low as possible, and made it compatible with Linux and Windows without any third party libraries, relying only on well established standards. This will make it compatible to the most systems even when compiling on a Linux machine and compile to Windows, and then running it on Linux using Wine without any type of tweaking.

For the graphics part, it makes use of a RAW display buffer only for graphics. And I have made a simple library that makes GUI's for audio plugins possible with the least amount of code and dependencies.

This library was originally made for making classic programming following the style of programming form the 80's. And it's tweaked to be even more flexible and easy to use than it's ever has been. All code made for making standalone applications is stripped form the library at this moment. The code for making this was originally made for SDL2, but as this lacks compatibility for child windows that is necessary for audio plugins, I made my own compatibility layer that replaces SDL2.

## The graphics library used
For painting graphics, it uses almost no code at all. And it has a separate file that connects to different platforms that is also small and different, depending on what platform the compiler uses.
For graphics it uses character maps, an old concept widely used in the 80's and in 2D computer games. But in this case you can place them anywhere in the editor window side by side. It will soon be fixed so you can layer them with alpha channel with a little tweak of the code.

Character maps is a very powerful concept, that gives very little code, for doing wonderous stuff. It can be used for animations and other stuff very easily and fits the needs for graphics in audio plugins very well. The posibility to make stand alone applications may be reintroduced later on, and also make the posiblility to make old school application programming and easy game development with the same ease of use later on.

## Tutorials
A tutorial on how to use and change this example will luckely be provided later on. This is only a showcase at the moment, even if it's fully working code.
## Compilation on Linux
It can easily be done on a standard Linux machine with a command like...
gcc generic_fx_code.c -o plugin.so -fPIC -shared
...for a 64-bit Linux plugin. Or...
x86_64-w64-mingw32-gcc generic_fx_code.c -o plugin.dll -fPIC -shared -lgdi32
...for a 64-bit Windows plugin.



