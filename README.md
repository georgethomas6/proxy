This is George Thomas Alexander's final project for Dr. Carl's Operating Systems course (CS428).

To compile the sequential proxy, without fsan, use the command `make sequential-unsanitized` in your commandline, likewise use the command `make sequential-sanitized` to compile it with fsan.
Both commands will compile it to an executable named `./sequential`.

To compile the multi-threaded proxy, without fsan, use the command `make multi-unsanitized` in your commandline, likewise use the command `make sequential-sanitized` to compile it with fsan.
Both commands will compile it to an executable named `./multi`.

## Pre-reqs

1. Must have my blocked.txt in the same directory as sequential_proxy.c and multi_proxy.c

## A note on the structure of the project

My helper files are all contained in the directory `./helpers`.
I was a bit over eager when I began this project, so I did not write tests for much of the code.
There is a `test.c` file in `./helpers` that tests the functionality of my `chunked_string`.
To compile it use the command `make test` in your commandline.
The Makefile will compile it to an executable named `./test`.
The bulk of the code for the proxy server is located in `sequential.c`.
The actual code for the sequential proxy server is located in `sequential_proxy.c`
The code for the multi-threaded proxy server is located in `multi_proxy.c`

## Known bugs and limitations

My final project could certainly have been planned better.
This is a tremendous case of if only I had known what I know now when I started.

1. There is AT LEAST one memory leak in the multi-threaded proxy server and the sequential proxy server.
   It becomes apparent when you run `make multi-sanitized` and make a request.
2. I do not log blocked requests.
   I got lazy and filtering was the last thing I did.
3. I finished this five minutes before the due date, so I did not have time to test it on the linux machines. It works on my machine though :)

## Conclusion

I am personally very proud of this final project.
This is by far the most complicated thing I have ever made and designed by myself, and I made it without the help of CSAPP.
Whether or not that was an intelligent decision is up for interpretation.
I am planning to build a similar project in the future for fun.
I was pushed very much by this project and I am proud this is the last thing I will turn in for my undergrad career.
