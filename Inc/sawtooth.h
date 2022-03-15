#include "dac.h"
#include "dma.h"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void sampleSound(); // Set values to be read to the DAC by the DMA

void init_sound(); // Start the sound on the piano

#ifdef __cplusplus
}
#endif