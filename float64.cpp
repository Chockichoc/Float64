#include "float64.h"
#include <arduino.h>

uint8_t countDigit(uint64_t n) 
{ 
    uint8_t count = 0; 
    while (n != 0) { 
        n = n / 10; 
        ++count; 
    } 
    
    return count; 
}

uint64_t power(uint64_t a, uint8_t b) {
  uint64_t result = 1;
  for(uint8_t i = 0; i < b; i++) {
    result *= a;
  }
  return result;
}

Float64::Float64(int64_t integerPart, uint64_t fractionalPart){
  /////SET SIGN BIT
  if(integerPart > 0)
    number &= ~((uint64_t)1 << 63);
  else {
    number |= (uint64_t)1 << 63;
    integerPart *= -1;
  }



  /////SET MANTISSA BITS
  uint8_t integerPartWidth = 52;
  while(!(integerPart & 0x8000000000000) && integerPartWidth > 0) {
    integerPart = integerPart << 1;
    integerPartWidth--;
  }


  
  Serial.print(integerPartWidth);


  uint8_t fractionalPartWidth = integerPartWidth;
  uint64_t binaryFractionalPart = 0;
  while(fractionalPart != 0 && fractionalPartWidth <= 50) {
    uint8_t digitCount = countDigit(fractionalPart);
    fractionalPart *= 2;
    uint8_t bit = (fractionalPart*2)/(power(10, digitCount));
    if(bit)
      binaryFractionalPart |= 1 << (50-fractionalPartWidth);
    fractionalPartWidth++;
  }

/*
  number += (uint64_t)integerPart;
  number += (uint64_t)binaryFractionalPart;

  uint64_t f = number >> 32;
  uint32_t b = f;
  Serial.println(b, BIN);
  b = number;
  Serial.println(b, BIN);
*/
  /////SET EXPONENT BITS

}


uint8_t Float64::getSign() {
  return number >> 63;
}

int16_t Float64::getExponent() {
  return (number & 0x7FF0000000000000) >> 52;
}

uint64_t Float64::getMantissa() {
  return number & 0x000FFFFFFFFFFFFF;
}
