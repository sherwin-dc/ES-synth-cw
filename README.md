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

## Tasks Performed by System
## Critical Instant Analysis of Scheduler
## Quantification of Total CPU Utilisation
## Shared Data Structures

### playedNotes

Information about which keys on the piano are being pressed is stored in a 108 element long uint8_t array called *playedNotes*. Though there are only 12 keys on each synthesizer, the *playedNotes* array is 108 elements long to make connecting multiple synthesizers together easier. The array elements give the states of each note in consecutive order from the deepest note to the highest note, starting with C0 at index 0 and going up to B8 at index 107. A zero entry means that the note is not being played a non-zero entry means that the note is being played.

The *playedNotes* array is accessed by the functions *sampleSound()*, *scanKeysTask()*, *Knob::update()*, *updateLCD()* and *decodeCANMessages()*. To ensure synchronisation between tasks the array has been declared using the keyword *volatile* which means that the variable will be accessed from memory, rather than from local registers, each time it is used. To guarantee safe access between threads the elements in the *playedNotes* array are always accessed using atomic operations. Specifically, we use the functions *__atomic_load_n()* and *__atomic_store_n()*.

### Volume, Octave, Reverb, Sound

### joystick.pitch

### Steps (in sawtooth.cpp)

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

DMA for DAC
