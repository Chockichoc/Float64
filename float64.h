#ifndef FLOAT64_H
#define FLOAT64_H

#include <Arduino.h>

class Float64 {
  private:
    uint8_t sign = 0;
    uint16_t exponent = 0;
    uint64_t mantissa = 0;
    
  public:
    Float64(uint8_t sign, uint16_t exponent, uint64_t mantissa) : 
      sign{sign}, exponent{exponent}, mantissa{mantissa} {};
    Float64(uint64_t _number);
    Float64(int32_t inputNumber, int8_t exponent);
    
    uint8_t getSign() const;
    uint16_t getExponent() const;
    uint64_t getMantissa() const;

    uint64_t getHexVersion();
    

    Float64 add(Float64& lOperand, Float64& rOperand);
    Float64 operator+(Float64& rOperand);
    Float64 operator-(Float64 rOperand);
    Float64 operator*(Float64& rOperand);
    Float64 operator/(Float64& rOperand);
    bool operator==(const Float64& rOperand);
    bool operator>(const Float64& rOperand);
    bool operator<(const Float64& rOperand);
};


#endif
