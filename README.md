# Documentation of Music Synthesiser

## Table of Contents
- [Demonstration Video](#demonstration-video)
- [Tasks Performed by System](#tasks-performed-by-system)
- [Critical Instant Analysis of Scheduler](#critical-instant-analysis-of-scheduler)
- [Quantification of Total CPU Utilisation](#quantification-of-total-cpu-utilisation)
- [Shared Data Structures](#shared-data-structures)
- [Analysis of Inter-task Blocking Dependencies](#analysis-of-inter-task-blocking-dependencies)
- [Advanced Features](#advanced-features)


## Demonstration Video

The following video showcases the functionality of the music synthesizer and highlights some of its advanced features.

STMCube is used instead of STMduino framework to exploit functionality of the stm32 board such as DMA which will be discussed in section *TODO*. HAL functions are called directly

## Tasks Performed by System
*talk about freeRTOS*

Below table shows an overview of the tasks that are performed and their corresponding sample rate and priority number. Lower priority numbers denote low priority tasks. (src: https://www.freertos.org/RTOS-task-priority.html)
| Task | Sample Rate (ms) | Priority Number |
| --- | --- | --- |
| scanKeysTask | 20 | 7 |
| updateLCD | 100 | 1 |

As `scanKeysTask` runs with a much higher frequency, it is assigned with a higher priority number compared to `updateLCD`.

*talks about DMA*
It was discovered that the processor was not able to deal with too much high intensive tasks that require reading and writing from the memory. Hence, DMA is utilised to ...


### scanKeysTask

The task reads the GPIO digital pins (C0 - C3) and determine the state of the keys and knobs on the module. It also writes to the GPIO pins (RA0 - RA2) which allow different "rows" to be read from the key matrix. It also decodes the rotation of the knob by a state transition table. As state `01` and `10` of the knob are between detents of the knobs ... (*actually why lol*)
As the sample rate is not high enough to detect all the transient states, the transition table below is adopted instead of that stated in the lab instruction:

| Previous {B,A} | Current {B,A} | Rotation Variable |
| --- | --- | --- |
| 00 | 01 | -1 |
| 00 | 10 | +1 |
| 11 | 01 | +1 |
| 11 | 10 | -1 |

The keys that are pressed and the rotation variable of the knobs are updated and stored in a shared data structure.

*The task is ran with 20ms such that transient states of the knobs can be captrued*

*TODO: CAN bus*

### updateLCD

The task reads the state of the keys, knobs and joystick from the shared datas and displays it on the OLED display. It is ran with a sample rate of 100ms such that the OLED refreshes at the same rate as specified in the specification. 

Data corresponding to each knob are printed on a specific coordinate on the display such that they are displayed on top fo the knob.

*insert image*

### sampleSound

The function is called as an iterrupt. It reads the state of the keys and *features* and produce the corresponding sound wave. *writes into the array*

### DMA for DAC (speaker)

### DMA for ADC (joystick)

To read both of the axis of the joystick, the ADC is configured to dual channel (scan) conversion mode. It reads the two channels (A0, A1) in one read and writes to the shared data.



## Critical Instant Analysis of Scheduler
## Quantification of Total CPU Utilisation


## Shared Data Structures

### playedNotes

Information about which keys on the piano are being pressed is stored in a 108 element long uint8_t array called `playedNotes`. Though there are only 12 keys on each synthesizer, the `playedNotes` array is 108 elements long to make connecting multiple synthesizers together easier. The array elements give the states of each note in consecutive order from the deepest note to the highest note, starting with C0 at index 0 and going up to B8 at index 107. A zero entry means that the note is not being played a non-zero entry means that the note is being played.

The `playedNotes` array is accessed by the functions `sampleSound()`, `scanKeysTask()`, `Knob::update()`, `updateLCD()` and `decodeCANMessages()`. To ensure synchronisation between tasks, the array has been declared using the keyword `volatile` which means that the variable will be accessed from memory, rather than from local registers, each time it is used. To guarantee safe access between threads the elements in the `playedNotes` array are always accessed using atomic operations. Specifically, we use the functions `__atomic_load_n()` and `__atomic_store_n()`.

### volume, octave, sound, and reverb

`volume`, `octave`, `sound` and `reverb` are uint8_t variables which store the volume, octave level, sound profile and amount of reverb of the synthesizer. These variables are accessed by the functions `sampleSound`, `updateLCD()`, `Knob::update()` and `CAN_TX()????`. To ensure synchronisation between tasks, each of the variables have been declared using the keyword `volatile` which means that the variable will be accessed from memory, rather than from local registers, each time it is used. To guarantee safe access between threads the variables are always accessed using atomic operations, through the functions `__atomic_load_n()` and `__atomic_store_n()`.

### joystick

### steps

`steps` is a 1100 element long uint32_t array.
uint32_t steps [1100]

### Queues (Is this what CAN uses? Does CAN use semaphores?)


## Analysis of Inter-task Blocking Dependencies
## Advanced Features

### Polyphony

### More synth sound profiles

### Reverb

### Pitch

### Modulation

### External recording

### Class for knobs

A class for the knobs (`Knob`) is implemented. It is initilaised with a pointer to the coressponding shared data (`volume`, `octave`, `sound`, `reverb`). The method `update` reads in the old state and new state of the knob and writes the rotation variable to the address of the pointer.

Such implementation gives a cleaner code in the `scanKeysTask` task. This can also support future extension such as switching functionality *(?)* in each knob, which can be easily implemented by changing the child pointer in the class.
