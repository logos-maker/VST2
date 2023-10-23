![](./gfx/thelay.png)  
_A sceenshot of the actual plugin example_
# Audio plugin with editor, for Windows and Linux
Use this as a template if you want a simple way to test out to make a VST plug and try it out.
The code crosscompiles with 100% identical code to Windows and Linux and make 64-bit and 32bit plugins.

If you use it as a toolkit for making audio plugins, then it's the most simple audio plugin toolkit you will ever find.
And it's many thusand times simpler than the popular toolkit for audio plugins.

It subrogates the VST2 license by only using the VST2 ABI. Yes! the VST2 format will be alive and kicking forever. 
This makes it possible to make plugins to use in the majority of all music programs.

This will make it possible to make a new plug even for beginners. And if you understand all of it, you will have gain deep knowleage of this the plugin format, that few people have.
Almost everything you need for a new plug is in one small file with less than 70 lines of code.
If you strip away all the filled in code that's for the delay, than it's less than 50 lines of code of the core of this toolkit.
The code is written entirely in C code, a language that is the most commonly used for audio applications for hardware, 
and it's a much simpler language than C++ to learn, and will also give the fastest possible plugs possible, with the highest amount of compatibility.

## Making your own plug
I would check out the plug_specific_code.c file and tweek that file. Make a change and compile and check out the difference by loading the plugin into a music program.

## Subrogating the VST2 licence - Making VST2 development availabe for anyone
To make this available this code-base only uses the VST2 ABI.

It's proven in court that you can't protect a API from common usage according to the the U.S. Supreme Court’s April 5 ruling in Google LLC v. Oracle America Inc. in 2021.
And it's certainly the case that you can't protect an ABI with copyright within all reasonable doubt.
And any resemblance of the original API is well motivated as it's not even prohibited to make use of the original API without any violation of any original VST2 licenses.

This makes compiled binaries for the most amount of music programs, with the least amount of code and dependencies.

I have kept the code size as low as possible, and made it compatible with Linux and Windows without any third party libraries, relying only on well established standards. 
This will make it compatible to the most systems even when compiling on a Linux machine and compile to Windows.
And if you are a Linux user, then you can test your Windows plugs running Wine on Linux without problems.

## Ikigui - The graphics library used
Is a cross platform GUI library for plugins. That uses almost no code at all.

You can find out more about it [here](https://github.com/logos-maker/ikiGUI)

The core concept in ikiGUI is simplicity, so it's not much to learn, and will give fewer compatibility issues with you plugins.

## Compilation on Linux
It can easily be done on a standard Linux machine with a command like...
```
gcc generic_fx_code.c -o plugin.so -fPIC -shared
```
...for a 64-bit Linux plugin. Or...
```
x86_64-w64-mingw32-gcc generic_fx_code.c -o plugin.dll -fPIC -shared -lgdi32
```
...for a 64-bit Windows plugin. To install the x86_64-w64-mingw32-gcc command run...
```
sudo apt -y install mingw-w64
```
If you want to compile 32bit plugins on a 64bit machine run...
```
sudo apt-get install gcc-multilib
```
and use the -m32 flag for GCC.  
And if you want to make 32bit Windows plugs use...
```
i686-w64-mingw32-gcc generic_fx_code.c -o plugin.dll -fPIC -shared -lgdi32
```
## Compilation on Windows
MinGW-w64 can be used to compile the code on Windows. I would recommend downloading [TDM-GCC](https://jmeubank.github.io/tdm-gcc/articles/2021-05/10.3.0-release) and downloading the installer named tdm64-gcc-10.3.0-2.exe Then after that you should be able to compile from the CMD command prompt. You can compile with the example with a command like...
```
gcc generic_fx_code.c -o plugin.dll -fPIC -shared -lGDI32
```
...to make it generate the plugin .dll for you. There is also many other ways to install MinGW-w64 for making 64bit plugs. If you want a cross platform IDE for developing you can mabe try out [CodeLite](https://codelite.org/) or [Code::Blocks](https://www.codeblocks.org/) or [EclipseIDE](https://eclipseide.org/)
or use a simple editor that can run commands as you don't need any build scripts to compile this. Examples of text editors that can run the single command needed for compilation is for example [Geany](https://www.geany.org/) or [Sublime Text](https://www.sublimetext.com/).
## Licences
The code will soon be dual-licensed for a small fee for closed source projects. More info later.
