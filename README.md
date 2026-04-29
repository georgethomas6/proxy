This is George Thomas Alexander's final project for Dr. Carl's Operating Systems course (CS428).

To compile the sequential proxy server use the command `make sequential` in your commandline.
The Makefile will compile it to an executable named `./sequential`.

To compile the multi-threaded proxy server use the command `make multi` in your commandline.
The Makefile will compile it to an executable named `./multi`.

## Pre-requisites

The only pre-requesites needed to run my code are the code itself.

## A note on the structure of the project

My helper files are all contained in the directory `./helpers`.
I was a bit over eager when I began this project, so I did not write tests for much of the code.
There is a `test.c` file in `./helpers` that tests the functionality of my `chunked_string`.
To compile it use the command `make test` in your commandline.
The Makefile will compile it to an executable named `./test`.
The bulk of the code for the proxy server is located in `sequential.c`.
The actual code for the sequential proxy server is located in `sequential_proxy.c`
The code for the multi-threaded proxy server is located in `multi_proxy.c`
