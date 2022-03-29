![alt text](https://st3.depositphotos.com/1005049/31711/v/600/depositphotos_317110512-stock-illustration-garbage-man-at-work-lorry.jpg)

# Flash GC Simulator

This simulator enables to run various garbage collection (GC) algorithms on an SSD memory layout. We implement a fully customizable infastructure that allows you to run your algorithm of choice on many different memory settings and record statistics such as number of erases and write amplification.

For full details about the theoretical model and the algorithms implemented in this simulator please read the [project report](https://github.com/Eyallotan/GC_Simulator/blob/main/Garbage%20Collection%20Algorithms%20for%20Flash%20Memories.pdf). In the report you will find all the needed prerequisites, along with a full breakdown of each algorithm, experiments results and more. 

## Installation

Follow these steps:

```bash
$ git clone https://github.com/dorsura/GC_Simulator.git
$ cd GC_Simulator
$ make
$ ./Simulator <enter command line parameters>
```

## Usage

### Command Line Parameters

These are the parameters you must set for each simulation:
1. Number of physical blocks (T)
2. Number of logical blocks (U)
3. Pages per block (Z)
4. Page size (in bytes)
5. Number of pages (N) 
6. Window flag
7. Data distribution
8. GC algorithm
9. Optional parameter: Filename to redirect output to 

* For window flag choose between ```window_on``` or ```window_off```. If you choose to turn on the window flag, you will be asked to choose the window size. 
* For data distribution parameter choose between ```uniform``` or ```hot_cold```. If you choose hot/cold distribution, you will be asked to choose the hot page percentage and the probability for a hot page.
* For GC algorithm choose between the following:
1. ```greedy```
2. ```greedy_lookahead```
3. ```generational```. If you choose this option you will be prompt to choose the number of generations. You should make sure that the number of generations is at least 1 and smaller than T-U (this will also be enforced by the simulator). In the [project report](https://github.com/Eyallotan/GC_Simulator/blob/main/Garbage%20Collection%20Algorithms%20for%20Flash%20Memories.pdf) you can find an deep dive analysis regarding the selection of the optimal number of generations a given simulation. We also implemented a heuristic function called OF (overloading factor). This heuristic function can be used to help you choose the best number of generations for your simulation based on the given parameters (T,U,Z). In order to use the OF heuristic, enter 0 when you are prompted to choose the number of generations for you simulation, and the OF function will be applied and choose the number of generations for you.
4. ```writing_assingment``` - This algorithm was only partially implemented since it did not improve the writing performance and we decided to proceed in different directions for the time being. Therefore it doesn't support all features such as window feature etc. You can check out the source code for full documentation and TODOS in this subject.

### Examples

```bash
$ ./Simulator 64 50 32 4096 100000 window_off uniform greedy
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         50
Pages/Block:            32
Page Size:              4096
Alpha:                  0.78125
Over Provisioning:      0.28
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           greedy

Starting Greedy Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 7394. Write Amplification: 2.36604

$ ./Simulator 64 50 32 4096 100000 window_off uniform greedy_lookahead
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         50
Pages/Block:            32
Page Size:              4096
Alpha:                  0.78125
Over Provisioning:      0.28
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           greedy_lookahead

Starting Greedy LookAhead Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 7170. Write Amplification: 2.29439

$ ./Simulator 64 52 32 4096 100000 window_off uniform generational
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         52
Pages/Block:            32
Page Size:              4096
Alpha:                  0.8125
Over Provisioning:      0.230769
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for heuristic selection):
2
Number of generations set to 2 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 8002. Write Amplification: 2.56052

$ ./Simulator 64 52 32 4096 100000 window_off uniform generational
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         52
Pages/Block:            32
Page Size:              4096
Alpha:                  0.8125
Over Provisioning:      0.230769
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           generational

Enter number of generations for Generational GC (Enter 0 for heuristic selection):
0
Using Overloading facror heuristic to select number of generations...
Number of generations set to 3 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 7926. Write Amplification: 2.53632

$ ./Simulator 64 52 32 4096 100000 window_on hot_cold generational
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         52
Pages/Block:            32
Page Size:              4096
Alpha:                  0.8125
Over Provisioning:      0.230769
Number of Pages:        100000
Page Distribution:      hot_cold
GC Algorithm:           generational

Please enter parameters for Hot/Cold memory simulation.
Enter the hot page percentage out of all logical pages in memory (0-100):
5
Enter the probability for hot pages (0-1):
0.9
Enter Window Size:
50000
Window size set successfully to n=50000.

Enter number of generations for Generational GC (Enter 0 for heuristic selection):
0
Using Overloading facror heuristic to select number of generations...
Number of generations set to 3 generations.

Starting Generational Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 10489. Write Amplification: 3.35652
```

### Window flag
We added a window flag option that let's you run simulations using a fixed window of future writes, while all other writes are generated on the spot (with no future knowledge). For example: 
```bash
$ ./Simulator 64 55 32 4096 10000 window_on uniform greedy_lookahead
Starting GC Simulator!
Physical Blocks:        64
Logical Blocks:         55
Pages/Block:            32
Page Size:              4096
Alpha:                  0.859375
Over Provisioning:      0.163636
Number of Pages:        10000
Page Distribution:      uniform
GC Algorithm:           greedy_lookahead

Enter Window Size:
2000
Window size set successfully to n=2000.

Starting Greedy LookAhead Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Simulation Results:
Number of erases: 1063. Write Amplification: 3.4016
```
In the example avoce, the simulation will first generate a unifromly distributed writing sequence with length of 2000 (a.k.a window size) and use Greedy Lookahead algorithm to write all the pages in the window. Note that the future knowledge is now bounded by the window size, i.e., when we reach write number 1995, we only have 5 future writes as reference, even though we still have 8000+ writes remaining for the whole simulation. After completing the writes within the window size, the simulation will write all remaining pages using the classic Greedy GC algorithm. 

This feature can be usefull for simulating the affect of write ahead buffers in RAM (non-volotile memory) that provide us with a short future of writes. This gives us the oppurtunity to benefit from using our algorithms. As n (the window size) approaches N (Total number of pages), we get a writing performance that approaches the performance of the improved algorithm (Greedy Lookahead / Generational). As n approaces 0 we get a performance that approaches the perforamnce of the classic Greedy GC. For more results and deep dive analysis see the [project report](https://github.com/Eyallotan/GC_Simulator/blob/main/Garbage%20Collection%20Algorithms%20for%20Flash%20Memories.pdf).

### Print Mode
We have implemented a print mode option that reflects block and page statistics as the simulator runs, along with information about the number of logical writes and more useful information. The print mode option is turned off by default and should not be used unless you redirect your output to a file (otherwise print time will probably make the simulation run for a very long time). if you wish to turn on the print mode you can comment out the following line in [```main.cpp```](main.cpp):
```cpp
110.  /* if you wish to activate print mode remove comment */
111.    //scg->setPrintMode(true);
``` 
Now when we run the simulator we'll get the relevant statistics.  
For exmaple (here we print to terminal just for the example. You should redirect to output file):
```bash
$ ./Simulator 12 7 4 4096 100000 window_off uniform greedy
Starting GC Simulator!
Physical Blocks:        12
Logical Blocks:         7
Pages/Block:            4
Page Size:              4096
Alpha:                  0.583333
Over Provisioning:      0.714286
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           greedy

Starting Greedy Algorithm simulation...
Reaching Steady State...
Steady State Reached...

Erases          Logical Writes  Y      V[0]    V[1]    V[2]    V[3]    V[4]
1               48              0       2       4       2       2       2
2               52              0       2       4       1       4       1
3               56              0       2       3       3       2       2
4               60              0       1       6       0       3       2
5               64              0       1       5       1       3       2
6               68              0       1       5       0       4       2
7               72              1       0       5       1       4       2
8               75              1       0       4       2       5       1
9               78              0       1       3       2       4       2
10              82              1       0       4       3       3       2
11              85              0       2       1       3       4       2
12              89              0       1       2       3       5       1
13              93              0       1       1       4       6       0
14              97              1       0       5       0       6       1
15              100             1       0       4       3       3       2
16              103             0       1       2       3       5       1
17              107             0       2       1       3       4       2
18              111             0       1       1       3       7       0
19              115             1       0       2       4       6       0
20              118             1       0       2       5       4       1
21              121             1       0       3       3       5       1
22              124             1       0       4       2       4       2
23              127             1       0       4       1       6       1
24              130             1       0       4       2       4       2
25              133             1       0       4       2       4       2
26              136             1       0       5       0       5       2
27              139             0       1       3       2       3       3
28              143             0       1       3       2       3       3
29              147             1       0       3       4       3       2^C

```

### Steady State 
We use steady state assumption (as expalined in the [project report](https://github.com/Eyallotan/GC_Simulator/blob/main/Garbage%20Collection%20Algorithms%20for%20Flash%20Memories.pdf)). Therefore the steady state flag is automatically turned on. if you wish to turn it off you can comment out the following line in [```main.cpp```](main.cpp):
```cpp
113.  /* if you wish to deactivate steady state mode remove comment */
114.    //scg->setSteadyState(false);
``` 
You can also adjust the number of writes done in order to achieve steady state (this can be better integrated in the future to be part of the adjustable parameters..). 

### Debug Mode
For your convenience, we implemented a simple memory layout printer. This can be used to print the memory layout as the simulator runs. 
Important: This feature is designed to work on small memory layouts. Make sure that U*Z < 100 in order to get a good looking result.
We may consider making this feature a bit more robust in the future, but we think that for debugging purposes this works fine for now.
To use this feature simply plant the function ```printMemoryLayout()``` implemented in [```FTL.hpp```](FTL.hpp).
This may look something like this:
```cpp
  void runGreedySimulation(Algorithm algo) {
        if (reach_steady_state){
            reachSteadyState();
        }
        for (unsigned long long i = 0; i < NUMBER_OF_PAGES; i++) {
        /* DEBUG mode - print memory layout */
            ftl->printMemoryLayout();
            ftl->write(data,writing_sequence[i],algo, writing_sequence, i);
        }
    }
```
Now when we run the simulation we will see the full memory layout, with logical block numbers for valid pages and X's for invalid pages:
```bash
$ ./Simulator 12 7 4 4096 100000 window_off uniform greedy
Starting GC Simulator!
Physical Blocks:        12
Logical Blocks:         7
Pages/Block:            4
Page Size:              4096
Alpha:                  0.583333
Over Provisioning:      0.714286
Number of Pages:        100000
Page Distribution:      uniform
GC Algorithm:           greedy

Starting Greedy Algorithm simulation...
Reaching Steady State...
Steady State Reached...

       0    1    2    3    4    5    6    7    8    9    10    11
     ------------------------------------------------------------
0   | X  |  2 | 26 | 11 | 24 | 18 |  0 | X  | 10 | X  | 12 | 20 |
     ------------------------------------------------------------
1   |  6 |  8 | 23 |  4 | X  | X  | X  | 19 | X  |  5 | X  |  1 |
     ------------------------------------------------------------
2   | 25 |  3 |  9 |    | 17 | X  | X  | 22 | X  | X  | 16 | 15 |
     ------------------------------------------------------------
3   | X  | 21 |  7 |    | X  | X  | X  | 27 | X  | 13 | X  | 14 |
     ------------------------------------------------------------
     
       0    1    2    3    4    5    6    7    8    9    10    11
     ------------------------------------------------------------
0   | X  |  2 | 26 | X  | 24 | 18 |  0 | X  | 10 | X  | 12 | 20 |
     ------------------------------------------------------------
1   |  6 |  8 | 23 |  4 | X  | X  | X  | 19 | X  |  5 | X  |  1 |
     ------------------------------------------------------------
2   | 25 |  3 |  9 | 11 | 17 | X  | X  | 22 | X  | X  | 16 | 15 |
     ------------------------------------------------------------
3   | X  | 21 |  7 |    | X  | X  | X  | 27 | X  | 13 | X  | 14 |
     ------------------------------------------------------------
     
       0    1    2    3    4    5    6    7    8    9    10    11
     ------------------------------------------------------------
0   | X  |  2 | 26 | X  | 24 | 18 |  0 | X  | X  | X  | 12 | 20 |
     ------------------------------------------------------------
1   |  6 |  8 | 23 |  4 | X  | X  | X  | 19 | X  |  5 | X  |  1 |
     ------------------------------------------------------------
2   | 25 |  3 |  9 | 11 | 17 | X  | X  | 22 | X  | X  | 16 | 15 |
     ------------------------------------------------------------
3   | X  | 21 |  7 | 10 | X  | X  | X  | 27 | X  | 13 | X  | 14 |
     ------------------------------------------------------------
       0    1    2    3    4    5    6    7    8    9    10    11
       
     ------------------------------------------------------------
0   | X  |  2 | 26 | X  | 24 | 18 |  0 | X  | 22 | X  | 12 | 20 |
     ------------------------------------------------------------
1   |  6 |  8 | 23 |  4 | X  | X  | X  | 19 |    |  5 | X  |  1 |
     ------------------------------------------------------------
2   | 25 |  3 |  9 | 11 | 17 | X  | X  | X  |    | X  | 16 | 15 |
     ------------------------------------------------------------
3   | X  | 21 |  7 | 10 | X  | X  | X  | 27 |    | 13 | X  | 14 |
     ------------------------------------------------------------^C
```

## Contributing

Pull requests are welcomed. 
There are many TODO's we hope to get to in the near (or far) future. 
Some of our ideas and afterthoughts are written in the [project report](https://github.com/Eyallotan/GC_Simulator/blob/main/Garbage%20Collection%20Algorithms%20for%20Flash%20Memories.pdf).

## License

All rights are reserved to Eyal Lotan and Dor Sura.
The Simulator FTL interface is based on the work of Alex Yucovich.

