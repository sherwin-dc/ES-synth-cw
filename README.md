# Documentation of Music Synthesiser

## Table of Contents
- [Documentation of Music Synthesiser](#documentation-of-music-synthesiser)
  - [Table of Contents](#table-of-contents)
  - [Demonstration Video](#demonstration-video)
  - [Tasks Performed by System](#tasks-performed-by-system)
    - [scanKeysTask](#scankeystask)
    - [updateLCD](#updatelcd)
    - [sampleSound](#samplesound)
    - [DMA for DAC (speaker)](#dma-for-dac-speaker)
    - [DMA for ADC (joystick)](#dma-for-adc-joystick)
  - [Critical Instant Analysis of Scheduler](#critical-instant-analysis-of-scheduler)
  - [Quantification of Total CPU Utilisation](#quantification-of-total-cpu-utilisation)
  - [Shared Data Structures](#shared-data-structures)
    - [playedNotes](#playednotes)
    - [volume, octave, sound, and reverb](#volume-octave-sound-and-reverb)
    - [joystick](#joystick)
    - [steps](#steps)
    - [Queues (Is this what CAN uses? Does CAN use semaphores?)](#queues-is-this-what-can-uses-does-can-use-semaphores)
  - [Analysis of Inter-task Blocking Dependencies](#analysis-of-inter-task-blocking-dependencies)
  - [Advanced Features](#advanced-features)
    - [Polyphony](#polyphony)
    - [Additional Synth Sound Profiles](#additional-synth-sound-profiles)
    - [Reverb](#reverb)
    - [Pitch](#pitch)
    - [Modulation](#modulation)
    - [External recording](#external-recording)
    - [Class for knobs](#class-for-knobs)


## Demonstration Video

The following video showcases the functionality of the music synthesizer and highlights some of its advanced features.

*insert video*

STMCube is used instead of STMduino framework to exploit functionality of the stm32 board such as DMA. HAL functions are called directly.

## Tasks Performed by System
*talk about freeRTOS*

DMA allows direct read/write to the memory. Some tasks were decided to implement using the DMA to reduce the load of the processor, as it was observed that the processor does not have sufficient capacity to deal with *too much high intesive tasks*. DMA is connected to the DAC and ADC directly to read and write from the analog pins on the MCU.

Below table shows an overview of the tasks that are performed and their corresponding sample rate and priority number. Lower priority numbers denote low priority tasks. (src: https://www.freertos.org/RTOS-task-priority.html)

| Task | Sample Rate (ms) | Priority Number |
| --- | --- | --- |
| scanKeysTask | 20 | 7 |
| updateLCD | 100 | 1 |
| DMA for ADC | 45.45 ± 5% | (DMA) |

As `scanKeysTask` runs with a much higher frequency, it is assigned with a higher priority number compared to `updateLCD`. The DMA for ADC has a variable sample rate which is implemented for pitch bend. Please refer to *section* for implementation details.

### scanKeysTask

The task reads the GPIO digital pins (C0 - C3) and determine the state of the keys and knobs on the module. It also writes to the GPIO pins (RA0 - RA2) which allow different "rows" to be read from the key matrix. It also decodes the rotation of the knob by a state transition table. The task is ran with 20ms sample rate such that transient states of the knobs can be captured in most use case.

As state `01` and `10` of the knob are between detents of the knobs, it is assumed that the next state must be the next detent in the same rotation direction. Hence the transition table below is adopted instead of that stated in the lab instruction. With such transition table, the corresponding data is only increment or decrement by one for ever detent. Although transitions cannot be correctly detected when the knobs are rotated fastly, it was decided that it is sufficient for majority of time.

| Previous {B,A} | Current {B,A} | Rotation Variable |
| --- | --- | --- |
| 00 | 01 | -1 |
| 00 | 10 | +1 |
| 11 | 01 | +1 |
| 11 | 10 | -1 |

The keys that are pressed and the rotation variable of the knobs are updated and stored in a shared data structure.

*TODO: CAN bus*

### updateLCD

The task reads the state of the keys, knobs and joystick from the shared datas and displays it on the OLED display. It is ran with a sample rate of 100ms such that the OLED refreshes at the same rate as specified in the specification. 

Data corresponding to each knob are printed on a specific coordinate on the display such that they are displayed on top fo the knob.

*insert image*

### sampleSound

The function is called as an iterrupt. It reads the state of the keys and *features* and produce the corresponding sound wave. *writes into the array*

### DMA for DAC (speaker)

### DMA for ADC (joystick)

To read both of the axis of the joystick, the ADC is configured to dual channel (scan) conversion mode. It reads the two channels (A0, A1) in one read and writes to the shared data in the memory via DMA.



## Critical Instant Analysis of Scheduler
## Quantification of Total CPU Utilisation


## Shared Data Structures

### playedNotes

Information about which keys on the piano are being pressed is stored in a 108 element uint8_t array called `playedNotes`. Though there are only 12 keys on each synthesizer, the `playedNotes` array is 108 elements long to make connecting multiple synthesizers together easier. The array elements give the states of each note in consecutive order from the deepest note to the highest note, starting with C0 at index 0 and going up to B8 at index 107. A zero entry means that the note is not being played a non-zero entry means that the note is being played.

The `playedNotes` array is accessed by the functions `sampleSound()`, `scanKeysTask()`, `Knob::update()`, `updateLCD()` and `decodeCANMessages()`. To ensure synchronisation between tasks, the array has been declared using the keyword `volatile` which means that the variable will be accessed from memory, rather than from local registers, each time it is used. To guarantee safe access between threads the elements in the `playedNotes` array are always accessed using atomic operations. Specifically, we use the functions `__atomic_load_n()` and `__atomic_store_n()`.

### volume, octave, sound, and reverb

`volume`, `octave`, `sound` and `reverb` are uint8_t variables which store the volume, octave level, sound profile and amount of reverb of the synthesizer. These variables are accessed by the functions `sampleSound`, `updateLCD()`, `Knob::update()` and `CAN_TX()????`. To ensure synchronisation between tasks, each of the variables have been declared using the keyword `volatile` which means that the variable will be accessed from memory, rather than from local registers, each time it is used. To guarantee safe access between threads the variables are always accessed using atomic operations, through the functions `__atomic_load_n()` and `__atomic_store_n()`.

### joystick

`joystick` is variable of type `ADC` which is a custom struct which has the member variables `pitch` and `modulation` which track the x-position and y-position of the joystick respectively. `pitch` and `modulation` are accessed by the function `sampleSound()` and a DMA. `sampleSound()` only reads from the variables, while the DMA only writes to the variables. To ensure that `sampleSound()` always uses the most up to date value of the variables, `joystick` has been declared as a volatile variable.

### steps

`steps` is a 600 element uint32_t array found in sawtooth.cpp. The array is accessed both by the `sampleSound()` function and a DMA which writes the values stored in the array to the DAC. As `sampleSound()` only writes to and never reads `steps` it is not necessary to declare the array as volatile. To guarantee safe access to the array the `sampleSound()` function and the DMA never operate on data in the same half of the array. Whenever the DMA reads the middle element of the `steps` array it triggers an interrupt which calls `sampleSound()` which then updates the first half of the array. Likewise, when the DMA reads the last element of the `steps` array, an interrupt is triggered which causes *sampleSound()* to update the second half of the `steps` array. This way, the `sampleSound()` function never updates elements in the part of the array which is currently being read by the DMA.

### Queues (Is this what CAN uses? Does CAN use semaphores?)


## Analysis of Inter-task Blocking Dependencies
## Advanced Features

### Polyphony

The first extension to the functionality of the synthesizer is Polyphony, i.e. being able to play multiple notes at the same time. Polyphony is enabled for all sound profiles except the standard sawtooth sound profile. Polyphony was implemented by first adding separate accumulators of type int32_t for all of the notes between C0 and B8. Whenever multiple notes are played at the same time, the values of the accumulators belonging to the notes being played are first shifted based on the current volume of the synth, then all added together. Finally, an offset is added to the sum of the accumulators to find which value should be written to the DAC. There is no limit on how many notes can be played at the same time and in theory all 108 notes between C0 and B8 can be played at the same time.

### Additional Synth Sound Profiles

1. #### Poly

This sound profile is identical to the standard sawtooth sound profile, except that polyphony is enabled when this sound profile is selected.

2. #### Chorus

The chorus sound profile gives the piano a "richer" sound. Chorus still uses sawtooth waveforms to produce different frequencies, however a note no longer consists of a single sawtooth waveform. Instead, each note consists of three sawtooth waveforms of slightly different frequencies. This produces a very pleasent sound profile and makes the piano sound like a choir.

3. #### Laser

The *Laser* sound profile is similar to the chorus sound profile, but now every note consists of 5 sawtooth waveforms of slightly different frequencies. The sound profile has been given the name "Laser" as pressing a note produces a sound similar to what laser blasters traditionally produce in movies. 

4. #### Sine

In the *Sine* sound profile the sawtooth waveforms have been replace by sine waves. Different frequency sine waves are produces by looping through an array, which holds one period of a sine wave, at different frequencies.

### Reverb

### Pitch

### Modulation

### External recording

### Class for knobs

A class for the knobs (`Knob`) is implemented. It is initilaised with a pointer to the coressponding shared data (`volume`, `octave`, `sound`, `reverb`). The method `update` reads in the old state and new state of the knob and writes the rotation variable to the address of the pointer.

Such implementation gives a cleaner code in the `scanKeysTask` task. This can also support future extension such as switching functionality *(?)* in each knob, which can be easily implemented by changing the child pointer in the class.