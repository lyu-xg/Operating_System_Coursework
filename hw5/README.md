### Memory Estimation

- use `make mem` to estimate memory for current system, which will print out the estimation, also saving time measurement data in a file.  

- use `make visualize_mem` to generate graph which can be used to confirm the result. 

- use `make mem_all` to estimate AND generate graph. 

- Note that the estimation is usually smaller than the physical memory because there are other processes occupying memory space hence making swapping happen sooner. 

- the estimation algorithm iteratively request more memories and read from each page, until at one iteration, time needed when up "significantly". We measure this significance by comparing the mean of the derivative of the time at each iteration. 

- each iteration requests for 65536 pages for sane time measurement.

- to measure time on different platforms, we first need to detect the environment and approach each platform differently, see `getRealTime.c` for details.

### Cache Estimation

- use `make cache` to measure cache performance. 

- use `make visualize_stride` to visualize runtime/stride relationships as discussed in the paper

- use `make visualize_arraysize` to visualize array_size/stride relationship to better illustrate cache performances.

- we use `-O3` compiler optimization to prevent loop overhead occupying our time measurement.

- we "stride" through the "array" 32 times for convenient and accurate time measurements.

- `*_stride_time.png` shows the runtime/stride relationship diagram as shown in the paper.

- cache levels, on the other hand, can be best inducted from `*_arraysize_stride.png`

- First machine: my macbook. my macbook has caches of size 32KB, 256KB and 8MB. We can directly see pattern from `mac_arraysize_stride.png`

- Second machine: CCIS Linux machine, CCIS machine has caches of size 32KB, 256KB and 20MB. We can directly see pattern from `ccis_arraysize_stride.png`