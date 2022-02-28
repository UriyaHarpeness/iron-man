# Iron Man

Welcome to Iron Man!

Who doesn't love C? And (potentially) malicious stuff?? And doing all of it from a Python code??? Who???? Well I do not
care one bit, I like it, so here.

Iron Man can do all that, and even much more, while flipping the bird to static analysis... Let's start doing some (
suspicious?) things.

## Getting Started

First off clone this project, after that you'll need to install a couple of requirements, and then you'll only need to
build it and enjoy the ride.

### Prerequisites

The packages required for building the project are:
gcc, cmake.

```shell script
apt install gcc cmake
```

The python code does not have any requirements.

### Configuring

* Logging can be configured [here](c/logging/logging.h).

### Building

After installing all the requirements, building the project is as straight-forward as it can be.

```
cd c  # The server C code folder.
cmake . -DCMAKE_BUILD_TYPE=Debug
make iron_man  # Iron Man's server.
make module_math  # Example Iron Man module commands shared object.
```

That's it!

The build also generates files in the python folder: `iron_man_config.json` for `iron_man`, and `module_config.json`
for `module_math`. They will be used later.

> **Note:** Setting `CMAKE_BUILD_TYPE` for cmake to `Debug` or `Release` changes certain internal behaviors in the
> server.

### Usage

Iron Man consists of two parts:

* The [server side](c), written in C.
* The [client side](python), written in Python.

The server side is not flexible after being compiled, and is simply run without any arguments:

```shell
./iron_man
```

From there you can expect to see output that looks similar to this:

```text
[2042/01/12 18:36:27] CRITICAL  | /home/user/iron_man/remote_c/main.c:main:20                                    | Started Iron Man
[2042/01/12 18:36:27] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:initialize_commands:94        | Initialized commands
[2042/01/12 18:36:27] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:110           | Created socket
[2042/01/12 18:36:27] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:120           | Bound socket
[2042/01/12 18:36:27] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:126           | Listening on socket
```

At this point the server is waiting for a connection from the client, the server goes back to accepting clients after
one has disconnected, until asked to stop with the commands `stop` or `suicide`.

The client is exposed in two ways:

* A [python class](python/iron_man.py) called IronMan.
* A [`cmd.CMD` subclass](python/iron_cmd.py) (a shell) which exposes an interface for the python class.

Here is an example where the client connects to the server, runs `ls`, and asks it to suicide using the python package:

```python
import pathlib
from iron_man import IronMan

iron_man = IronMan(pathlib.Path('python/iron_man_config.json'))
print('Child exited with', iron_man.run_shell('ls', ['/']))
iron_man.suicide()
```

It produced the following output:

```text
Enter "KILL!" to kill the process.
========================= got =========================
bin
boot
dev
etc
home
lib
lib32
lib64
libx32
media
mnt
opt
proc
root
run
sbin
srv
sys
tmp
usr
var

-------------------------------------------------------
Child process died.
Child exited with 0
```

And here is the matching output from the server:

```text
[2042/01/12 18:55:14] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:134           | Accepted connection
[2042/01/12 18:55:14] WARNING   | /home/user/iron_man/remote_c/main.c:main:33                                    | Connected
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x712a339fb8ce86fd
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:27       | Running file: ls, with 1 args
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:99       | Created child process: 2139
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 119 bytes
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 0 bytes
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:138      | Child process has exited with status: 0
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:182      | Finished running file: ls
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x712a339fb8ce86fd
[2042/01/12 18:55:14] INFO      | /home/user/iron_man/remote_c/communication/connection.c:communicate:219        | Gracefully suiciding Iron Man
[2042/01/12 18:55:14] ERROR  10 | /home/user/iron_man/remote_c/communication/connection.c:communicate:224        | Successfully unlinked self
[2042/01/12 18:55:14] WARNING   | /home/user/iron_man/remote_c/main.c:main:35                                    | Disconnected
[2042/01/12 18:55:14] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_module_commands:399   | Destroyed modules
[2042/01/12 18:55:14] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_module_commands:399   | Destroyed modules
[2042/01/12 18:55:14] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_commands:408          | Destroyed commands
[2042/01/12 18:55:14] CRITICAL  | /home/user/iron_man/remote_c/main.c:main:52                                    | Stopped Iron Man
```

And this is exactly the same operations being run from the Iron Man shell:

```text
Welcome to Iron Man's shell.   Type help or ? to list commands.

(iron man) connect python/iron_man_config.json
(iron man) run_shell ls /
Enter "KILL!" to kill the process.
========================= got =========================
bin
boot
dev
etc
home
lib
lib32
lib64
libx32
media
mnt
opt
proc
root
run
sbin
srv
sys
tmp
usr
var

-------------------------------------------------------
Child process died.
Exit code: 0
(iron man) suicide
(iron man) exit
```

A more complicated example can be found [here](python/example.py) (it also includes the shell equivalent line for every
operation).

<details>
<summary><b>Running <code>python/example.py</code></b></summary>
<p>

Running the server:

```shell
./iron_man
```

Running the example client usage:

```shell
python python/examply.py
```

The example client usage output:

```text
Summing 4 and 4 gives: 8
Summing 4 and 400 gives: 404
Summing 1000 and 24 gives: 1024
The difference between 4 and 4 is: 0
The difference between 4 and 400 is: 396
The difference between 1000 and 24 is: 976
Enter "KILL!" to kill the process.
========================= got =========================
[2042/01/12 06:55:34] ERROR   2 | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:81       | Failed executing file nonexistant

-------------------------------------------------------
Child process died.
Running "nonexistant", exited with 128
Enter "KILL!" to kill the process.
Child process died.
Running "sleep 1", exited with 0
Enter "KILL!" to kill the process.
========================= got =========================
total 0
dr-xr-xr-x  9 root root     0 Jan 12 09:30 1
dr-xr-xr-x  9 user user     0 Jan 12 20:39 10
dr-xr-xr-x  9 root root     0 Jan 12 18:13 1950
dr-xr-xr-x  9 user user     0 Jan 12 20:39 1952
dr-xr-xr-x  9 user user     0 Jan 12 18:13 1953
dr-xr-xr-x  9 user user     0 Jan 12 19:17 2229
dr-xr-xr-x  9 user user     0 Jan 12 06:55 2364
dr-xr-xr-x  9 user user     0 Jan 12 06:55 2367
dr-xr-xr-x  9 user user     0 Jan 12 09:34 35
dr-xr-xr-x  9 root root     0 Jan 12 09:33 8
drwxrwxrwt  2 root root    40 Jan 12 09:30 acpi
-r--r--r--  1 root root     0 Jan 12 20:40 buddyinfo
dr-xr-xr-x  4 root root     0 Jan 12 09:30 bus
-r--r--r--  1 root root     0 Jan 12 20:40 cgroups
-r--r--r--  1 root root     0 Jan 12 20:40 cmdline
-r--r--r--  1 root root 28392 Jan 12 20:40 config.gz
-r--r--r--  1 root root     0 Jan 12 20:40 consoles
-r--r--r--  1 root root     0 Jan 12 20:40 cpuinfo
-r--r--r--  1 root root     0 Jan 12 20:40 crypto
-r--r--r--  1 root root     0 Jan 12 20:40 devices
-r--r--r--  1 root root     0 Jan 12 20:40 diskstats
-r--r--r--  1 root root     0 Jan 12 20:40 dma
drwxr-xr-x  6 root root     0 Jan 12 20:40 docker
dr-xr-xr-x  4 root root     0 Jan 12 20:40 driver
-r--r--r--  1 root root     0 Jan 12 20:40 execdomains
-r--r--r--  1 root root     0 Jan 12 20:40 fb
-r--r--r--  1 root root     0 Jan 12 09:30 filesystems
dr-xr-xr-x  8 root root     0 Jan 12 09:30 fs
-r--r--r--  1 root root     0 Jan 12 20:40 interrupts
-r--r--r--  1 root root     0 Jan 12 20:40 iomem
-r--r--r--  1 root root     0 Jan 12 20:40 ioports
dr-xr-xr-x 30 root root     0 Jan 12 09:30 irq
-r--r--r--  1 root root     0 Jan 12 20:40 kallsyms
crw-rw-rw-  1 root root  1, 3 Jan 12 09:30 kcore
-r--r--r--  1 root root     0 Jan 12 20:40 key-users
crw-rw-rw-  1 root root  1, 3 Jan 12 09:30 keys
-r--------  1 root root     0 Jan 12 20:40 kmsg
-r--------  1 root root     0 Jan 12 20:40 kpagecgroup
-r--------  1 root root     0 Jan 12 20:40 kpagecount
-r--------  1 root root     0 Jan 12 20:40 kpageflags
-r--r--r--  1 root root     0 Jan 12 20:40 loadavg
-r--r--r--  1 root root     0 Jan 12 20:40 locks
-r--r--r--  1 root root     0 Jan 12 20:40 meminfo
-r--r--r--  1 root root     0 Jan 12 20:40 misc
-r--r--r--  1 root root     0 Jan 12 20:40 modules
lrwxrwxrwx  1 root root    11 Jan 12 20:40 mounts -> self/mounts
dr-xr-xr-x  4 root root     0 Jan 12 20:40 mpt
-rw-r--r--  1 root root     0 Jan 12 20:40 mtrr
lrwxrwxrwx  1 root root     8 Jan 12 20:40 net -> self/net
-r--------  1 root root     0 Jan 12 20:40 pagetypeinfo
-r--r--r--  1 root root     0 Jan 12 20:40 partitions
crw-rw-rw-  1 root root  1, 3 Jan 12 09:30 sched_debug
lrwxrwxrwx  1 root root     0 Jan 12 09:30 self -> 2367
-rw-------  1 root root     0 Jan 12 20:40 slabinfo
-r--r--r--  1 root root     0 Jan 12 20:40 softirqs
-r--r--r--  1 root root     0 Jan 12 20:40 stat
-r--r--r--  1 root root     0 Jan 12 20:40 swaps
dr-xr-xr-x  1 root root     0 Jan 12 09:30 sys
--w-------  1 root root     0 Jan 12 09:30 sysrq-trigger
dr-xr-xr-x  5 root root     0 Jan 12 20:40 sysvipc
lrwxrwxrwx  1 root root     0 Jan 12 09:30 thread-self -> 2367/task/2367
crw-rw-rw-  1 root root  1, 3 Jan 12 09:30 timer_list
dr-xr-xr-x  6 root root     0 Jan 12 20:40 tty
-r--r--r--  1 root root     0 Jan 12 20:40 uptime
-r--r--r--  1 root root     0 Jan 12 20:40 version
-r--------  1 root root     0 Jan 12 20:40 vmallocinfo
-r--r--r--  1 root root     0 Jan 12 20:40 vmstat
-r--r--r--  1 root root     0 Jan 12 20:40 zoneinfo

-------------------------------------------------------
Child process died.
Running "ls -l /proc", exited with 0
Enter "KILL!" to kill the process.
Child process died.
Child exited with 0
127.0.0.1	localhost
::1	localhost ip6-localhost ip6-loopback
fe00::0	ip6-localnet
ff00::0	ip6-mcastprefix
ff02::1	ip6-allnodes
ff02::2	ip6-allrouters
172.17.0.2	d0e6ad50f39c

Getting file "/root"
Got failed result: FAILED_OPEN[EACCES - Permission denied]
Getting file "nonexistant"
Got failed result: FAILED_STAT[ENOENT - No such file or directory]
```

The output of the server:

```text
[2042/01/12 06:55:32] CRITICAL  | /home/user/iron_man/remote_c/main.c:main:20                                    | Started Iron Man
[2042/01/12 06:55:32] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:initialize_commands:94        | Initialized commands
[2042/01/12 06:55:32] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:110           | Created socket
[2042/01/12 06:55:32] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:120           | Bound socket
[2042/01/12 06:55:32] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:126           | Listening on socket
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/communication/connection.c:connect_:134           | Accepted connection
[2042/01/12 06:55:34] WARNING   | /home/user/iron_man/remote_c/main.c:main:33                                    | Connected
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:add_module_command:116        | Adding module command: /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so:sum, id: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/module.c:module_constructor:18            | Loading module
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:add_module_command:159        | Added module command: /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so:sum, id: 0x616a768f
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:add_module_command:116        | Adding module command: /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so:difference, id: 0x5685c271
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:add_module_command:159        | Added module command: /home/user/iron_man/remote_c/cmake-build-debug---remote-host/libmodule_math.so:difference, id: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:14                         | Summing numbers: 4 + 4
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:26                         | Summing result: = 8
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:14                         | Summing numbers: 4 + 400
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:26                         | Summing result: = 404
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:14                         | Summing numbers: 1000 + 24
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/sum.c:sum:26                         | Summing result: = 1024
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x616a768f
[2042/01/12 06:55:34] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:remove_module_command:356     | Removed module command: 0x616a768f
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:14           | Differencing numbers: |4 - 4|
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:26           | Difference result: = 0
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:14           | Differencing numbers: |4 - 400|
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:26           | Difference result: = 396
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:14           | Differencing numbers: |1000 - 24|
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/modules/math/difference.c:difference:26           | Difference result: = 976
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x5685c271
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xb42b2eb53d55d89c
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:5           | Getting file: /etc/hosts
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:31          | Got file: /etc/hosts
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0xb42b2eb53d55d89c
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xf568c06990e0a533
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:27       | Running file: nonexistant, with 0 args
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:99       | Created child process: 2365
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 149 bytes
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 0 bytes
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:138      | Child process has exited with status: 128
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:182      | Finished running file: nonexistant
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0xf568c06990e0a533
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xf568c06990e0a533
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:27       | Running file: sleep, with 1 args
[2042/01/12 06:55:34] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:99       | Created child process: 2366
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 0 bytes
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:138      | Child process has exited with status: 0
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:182      | Finished running file: sleep
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0xf568c06990e0a533
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xf568c06990e0a533
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:27       | Running file: ls, with 2 args
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:99       | Created child process: 2367
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 3479 bytes
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 0 bytes
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:138      | Child process has exited with status: 0
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:182      | Finished running file: ls
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0xf568c06990e0a533
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0x6e61763eea8e25d2
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/put_file/put_file.c:put_file:11          | Putting file: /tmp/hosts_copy
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/put_file/put_file.c:put_file:25          | Put file: /tmp/hosts_copy
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0x6e61763eea8e25d2
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xf568c06990e0a533
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:27       | Running file: rm, with 1 args
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:99       | Created child process: 2368
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:128      | Read output of 0 bytes
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:138      | Child process has exited with status: 0
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/run_shell/run_shell.c:run_shell:182      | Finished running file: rm
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:310               | Finished running command: 0xf568c06990e0a533
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xb42b2eb53d55d89c
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:5           | Getting file: /root
[2042/01/12 06:55:35] ERROR  13 | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:23          | Failed opening file: /root
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/commands.c:run_command:270               | Running command: 0xb42b2eb53d55d89c
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:5           | Getting file: nonexistant
[2042/01/12 06:55:35] ERROR   2 | /home/user/iron_man/remote_c/commands/get_file/get_file.c:get_file:13          | Failed getting information of file: nonexistant
[2042/01/12 06:55:35] ERROR   9 | /home/user/iron_man/remote_c/communication/connection.c:communicate:214        | Gracefully stopping Iron Man
[2042/01/12 06:55:35] WARNING   | /home/user/iron_man/remote_c/main.c:main:35                                    | Disconnected
[2042/01/12 06:55:35] INFO      | /home/user/iron_man/remote_c/modules/module.c:module_destructor:37             | Unloading module
[2042/01/12 06:55:35] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_module_commands:399   | Destroyed modules
[2042/01/12 06:55:35] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_module_commands:399   | Destroyed modules
[2042/01/12 06:55:35] DEBUG     | /home/user/iron_man/remote_c/commands/commands.c:destroy_commands:408          | Destroyed commands
[2042/01/12 06:55:35] CRITICAL  | /home/user/iron_man/remote_c/main.c:main:52                                    | Stopped Iron Man
```

</p>
</details>

## Technologies and Capabilities

* Iron Man is written in [C](https://en.wikipedia.org/wiki/C_(programming_language)) for the server side, and
  [Python](https://en.wikipedia.org/wiki/Python_(programming_language)) 3 for the client side.
* Features a shell which exposes an interface for the python class for easy immediate usage, with help and documentation
  for all operations.
* Supports logging to a file or stdout in a stylish format with different levels.
* Logs all errors, and sends back to the client when connected, using a result structure that contains the high level
  error, as well as the errno value, for full visibility of the failure reason.
* [Very hard to be statically analyzed](#Protection-and-Anonymization).
* Exposes an interactive run shell command, meaning that for example it can run bash, and pass the input into it, as
  well as supports killing the run shell command in the middle of execution (meaning it's non-blocking).
* Comes with a [Dockerfile](c/Dockerfile) that can be used to compile and run the server side (was used for development
  over MacOS).

### Exposed Capabilities

Here is a list of operations that Iron Man allows:

1. Get file command [`get_file`] - reads a file in a specified path.
2. Put file command [`put_file`] - writes a file in a specified path with given content.
3. Run shell command [`run_shell`] - runs a command with given arguments.
4. Add module command [`add_module_command`] - adds a command from a shared object file by path and function name.
5. Run module command [`run_module_command`] - runs a command added with the add module command by function name, with
   optional arguments to match the module command.
6. Remove module command [`remove_module_command`] - removes a module command by function name.
7. Disconnect [`disconnect`] - disconnect from the server, the server does back to accepting new clients.
8. Stop [`stop`] - disconnect from the server and stop the server accept loop, causing the server to exit.
9. Suicide [`suicide`] - disconnect from the server, delete the server executable, and stop the server accept loop,
   causing the server to exit.

`get_file`, `put_file`, and `run_shell` are commands called "builtin commands", they are the basic commands Iron Man
provides. Using `add_module_command`" adds "module commands", which are extensions to the builtin commands, and they are
reset for every connection.

### Protection and Anonymization

The server code has several features that help protect it and keep it anonymous:

1. All commands' (builtin and module) code is encrypted using AES encryption, the code is decrypted only when asked to
   be called and encrypted after finished. **The key and IV used to encrypt and decrypt the code are in the build
   generated config files (randomized for each compilation), meaning that without catching the code mid-execution, this
   code is impossible to be analyzed.**
2. All communication is encrypted, with each Iron Man build having its own AES key and IV, as well as having a unique
   generated handshake value required for accepting the connection.
3. Builtin commands' IDs are also generated for each compilation.
4. All string literals used in the server are encrypted using AES, and decrypted only at runtime.
5. Release build removes all logging, meaning all the logging strings and code are removed from the executable.
6. All system and library calls are dynamically loaded at runtime for release builds, and their names are encrypted.
7. Release build strips the binary, as well as removes these sections: `.comment`, `.note.ABI-tag`, `.gnu.hash`
   , `.gnu.version`, and `*note.gnu*`.
8. The following are generated for each build:
    1. The AES key and IV used to encrypt and decrypt the commands (do not appear in the server, must be passed to call
       the commands).
    2. The IDs of the builtin commands.
    3. The AES key and IV used for the communication.
    4. The handshake.
    5. The AES key and IV used to encrypt and decrypt the string literals.
    6. The AES key and IV used to encrypt and decrypt the system and library calls' names.

   > The python scripts that help generate all these can be found [here](python/building).

> These also apply for the example module `module_math`.

This results in:

1. No one can connect and communicate with an Iron Man server without having its build info (generated in compilation).
2. Communication is not understandable to any third party.
3. The code of commands cannot be decompiled if not caught in the middle of a successful execution requested from the
   client, so `objdump -D` for example will not be able to understand their code.
4. The `string` command is rendered useless, as string literals are encrypted, and system and library calls do not show
   there, as they are loaded at runtime, using their encrypted names.
5. Commands like `nm -D` or `readelf -s` are also no good here, since the system and library calls are dynamically
   loaded at runtime, their names do not appear in the executable.
6. All debug information is lost, as well as the addresses of functions, since the files is stripped and certain
   sections are removed.

All of the above makes static analysis very un-effective on Iron Man. Which is the whole point here.

> **The reason for the name:** Iron Man - iron is a hard metal, like this tool being hard to statically analyze. And
> also my favorite superhero, RIP.

## Documentation

The C++ code is thoroughly documented, using
[LSST DM Developer Guide](https://developer.lsst.io/cpp/api-docs.html#documenting-c-code) format.

The Python code is documented using [Google Python Style Guide](https://google.github.io/styleguide/pyguide.html)
format.

I hope that it can answer whichever questions that may arise.

## Contributing

Feel free to contact me if you have any comments, questions, ideas, and anything else you think I should be aware of. Of
course that any way Iron Man was used to mess with friends (under their consent) is also interesting to me. Plus any
cool module commands that were built to further extend Iron Man.

## Authors

* [**Uriya Harpeness**](https://github.com/UriyaHarpeness)

## Acknowledgments

* I would like to thank my wife - Tohar Harpeness, my son - Amittai Harpeness, my parents, my computer, and my free
  time, for enabling me to work on this small project, it has been fun.

* I thank [Linux](https://en.wikipedia.org/wiki/Linux) for being so fun and cooperative in terms of interaction with C
  code and its documentation, this would not be possible otherwise (ehm ehm Windows).

* Acknowledgment is also in order for my older and bygone iteration of the Iron Man concept, which also had the same
  name. Past me, this is for you.
