ShawnOS
======

ShawnOS is an Operating System/Kernel meant to help learn the fundamentals of Operating System development, primarily on Intel x86-based architectures.

The project is very much a work-in-progress at this point and many design decisions regarding the OS are yet to be made. Eventually, the project aims to reach a basic level of usability, with a functional console environment and C compiler.


Building
------
In order to build and run ShawnOS, a gcc cross-compiling toolchain must be present for the desired target (currently i686-elf is only supported).

The OSDev Wiki has a tutorial on how to create the required toolchain for Linux, OS X, and Windows [here](http://wiki.osdev.org/GCC_Cross-Compiler).

In addition to the tools mentioned in the OSDev wiki, you also need the `nasm` and `xorriso` packages in order to build ShawnOS. These are available in most Linux distributions' package managers.
```
# Ubuntu
$ sudo apt-get install nasm xorriso

# Fedora
$ sudo dnf install nasm xorriso
```


Once you have set up the necessary toolchain, you must update the `PREFIX` and `PATH` variables in `env.sh` to reflect its location.

Example:
```
export PREFIX='$HOME/opt/cross'
export TARGET=i686-elf
export PATH='$PREFIX/bin:$PATH'
```

Once you have completed this, simply run the following to load the variables:
```
$ source env.sh
```


Now you may build an ISO image of the OS:
```
$ ./iso.sh
```

If you have qemu installed for the proper architecture, you can also run ShawnOS in a virtual machine:
```
$ ./qemu.sh
```

Goals
------

The project eventually aims to provide a basic usable user space with console utilities and a C compiler.

Resources
------
Much of the code from ShawnOS is adapted from or inspired by the following resources:

[The OSDev Wiki](https://wiki.osdev.net)
[Bran's Kernel Development Tutorial](http://www.osdever.net/bkerndev/Docs/title.htm)
