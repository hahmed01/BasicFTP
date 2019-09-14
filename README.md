# BasicFTP

The server (C) handles sending a data file by using C strings or sending the contents of its directory. The client with (Python), provides input validation of commands and displays contents of the requested file or directory. Both host maintain their connection using sockets.

### Instructions for Running
These programs are meant to be executed on the Oregon State University's ENGR FLIP servers.

1) Compiling the server, using flip1.engr.oregonstate.edu
```
gcc -g fserver.c -o fserver
fserver <port number>
```


2) Starting up fclient.py, using flip2.engr.oregonstate.edu

```
python fclient.py flip1 <server port(same as fserver)> <COMMAND (-g followed by <FILENAME> OR -l)> <server port + 1>
```

###### Examples of running the programs with proper commands: 
To retreive file: 
```
python fclient.py flip1 4000 -g filename.txt 4001
```
To retrive directory: 
```
python fclient.py flip1 4000 -l 4001
```


