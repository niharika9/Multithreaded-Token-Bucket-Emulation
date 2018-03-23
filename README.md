# warmup2
Multi-threaded Token Bucket Emulation in C
Emulated a traffic shaper which transmits packets controlled by a token bucket using multi-threading.

Utilized 5 thread for packet arrival, token arrival, two servers, and a signal handling thread.
Used mutex for synchronization and conditional variable to avoid busy-wait.

Compile :
make or make warmup2

Run :
./warmup2 filename
