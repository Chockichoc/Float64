#ifndef FLOAT64_H
#define FLOAT64_H

#include <arduino.h>

class Float64 {
  private:
    uint64_t number = 0LL;
    
  public:
    Float64(uint64_t _number) : number{_number} {};
    Float64(int64_t integerPart, uint64_t fractionalPart);
    uint8_t getSign();
    int16_t getExponent();
    uint64_t getMantissa();
};


#endif
