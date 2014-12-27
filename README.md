Minesweeper
===========

Minesweeper written in C-language


## Usage

```sh
$ ./minesweeper
```


## Build

Use [Makefile](Makefile).

```sh
$ make
```

If you want to build with MSVC, use [msvc.mk](msvc.mk).
[msvc.mk](msvc.mk) is written for nmake.

###### Release version

```sh
> nmake /f msvc.mk
```

###### Debug version

```sh
> nmake /f msvc.mk DEBUG=true
```


## LICENSE

This software is released under the MIT License, see [LICENSE](LICENSE).
