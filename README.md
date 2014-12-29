Minesweeper
===========

Minesweeper written in C-language


## Usage

```sh
$ ./minesweeper [options]
```

#### Options

- ```-c COLUMN_SIZE```, ```--column=COLUMN_SIZE```
  - Specify column size
- ```-h```, ```--help```
  - Show help and exit
- ```-l LEVEL```, ```--level=LEVEL```
  - Specify level of minesweeper
  - **Levels**
    - easy:
      - row size is 9,  column size is 9  and number of mine is 10
    - normal:
      - row size is 16, column size is 16 and number of mine is 40
    - hard:
      - row size is 16, column size is 30 and number of mine is 99
- ```-m MODE```, ```--mode=MODE```
  - Specify mode
  - **Modes**
    - cursor:
      - You can move cursor
        - 'h': Move cursor left
        - 'j': Move cursor down
        - 'k': Move cursor up
        - 'l': Move cursor right
        - 'o': Open a panel under the cursor
        - 'f': Flag a panel under the cursor
    - prompt:
      - Show prompt and you have to input command and coordinate
      - [Command]
        - open: Open a panel
        - flag: Flag a panel
- ```-n N_MINE```, ```--n-mines=N_MINE```
  - Specify the number of mines
- ```-r ROW_SIZE```, ```--row=ROW_SIZE```
  - Specify row size



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


## Dependent libraries

- [TermUtil](https://github.com/koturn/TermUtil)

#### MSVC only

- [getopt clone](https://github.com/koturn/getopt)
- [msvcdbg](https://github.com/koturn/msvcdbg)


## LICENSE

This software is released under the MIT License, see [LICENSE](LICENSE).
