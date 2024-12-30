# Prexion

This repository contains the source codes of the Prexion compressor. The Prexion compressor has been implemented based on [zlib] (https://github.com/madler/zlib).

# Usage

The following instructions assume that this repository has been cloned into the `$PREXION_ROOT` directory.
These instructions have been tested on 64-bit Ubuntu 24.04 LTS with the `build-essential` package (including `gcc`, `make`, etc.) installed.

### Compression

The following commands will build the source codes and run the compression process for a given file.
The sample web pages are available in $PREXION_ROOT/webpage_sample.
The path and name of an input file should be given.
Additionally, a matching probability for Prexion should be also given; for example, $20$ shoud be given in order to apply $p=0.20$.
The resulting .gz file will be generated in the same directory as the input file.

```shell
$ cd $PREXION_ROOT
$ python3 ./compress.py [file to compress] [prexion matching probability (e.g., 20 for p=0.20)]
```
### Decompression

The following commands will run the decompression process for a given .gz file.
The recovered file will be generated in the same directory as the input file.

```shell
$ cd $PREXION_ROOT
$ ./minigzip -d [.gz file to decompress]
```
