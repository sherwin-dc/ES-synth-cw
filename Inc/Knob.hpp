#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

class Knob 
{
  public:
  Knob();
  Knob(volatile uint8_t* _target);
  void setTarget(volatile uint8_t* _target);
  void update(uint8_t oldState, uint8_t newState);

  private: 
  volatile uint8_t* target; // pointer to the parameter that the knob is tuning
  int8_t change(uint8_t oldState, uint8_t newState);
};

extern Knob knobs[4];

#ifdef __cplusplus
}
#endif