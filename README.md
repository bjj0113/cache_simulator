# cache_simulator

----

./cachesim -a=2 -s=64 -b=8 -f=sample.trc

a = cnt in index

b =  block size

s = memory size

f = filename

----

##sample.trc
00000000 R

00000004 R

00000008 W 2

0000000C W 3

00000010 W 4

00000014 W 5

00000018 R

0000001C R

00000020 R

00000024 R

----

##Ex)

gcc cachesim.c -o cachesim -Wall

./cachesim -a=2 -s=64 -b=8 -f=sample.trc

0: 00000058 00000061 v:1 d:1
   00000000 00000000 v:1 d:0 
1: 00000062 00000063 v:1 d:1
   0000000A 0000000B v:1 d:1   
2: 0000005C 00000055 v:1 d:1
   00000000 0000000D v:1 d:1
3: 0000005E 0000004F v:1 d:1
   00000000 00000000 v:1 d:0
total number of hits : 50
total number of misses: 50
miss rate: 50.0%
total number of dirty block: 6
average memory access cycle: 100.5
