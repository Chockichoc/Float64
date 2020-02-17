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

Float64::Float64(uint64_t number) {
  sign = number >> 63;
  mantissa = number & 0xFFFFFFFFFFFFF;
  exponent = (number >> 52) & 0x7FF;
}

Float64::Float64(int32_t inputNumber, int8_t exponent){

  /////SET SIGN BIT
  if(inputNumber > 0)
    sign = 0;
  else {
    sign = 1;
    inputNumber *= -1;
  }

  uint64_t integerPart = 0;
  uint64_t fractionalPart = 0;
  uint64_t baseNumber = 0;
  bool isFirstDigitSet = false;

  if(exponent < 0){
    baseNumber =  power(10, countDigit(inputNumber) - exponent - 1);
    integerPart = 0;
    fractionalPart = inputNumber;
  } else {
    isFirstDigitSet = true;
    if(exponent >= countDigit(inputNumber)){
      baseNumber = power(10, exponent - countDigit(inputNumber) + 1);
      integerPart = inputNumber*baseNumber;
      fractionalPart = 0;
    } else {
      baseNumber = power(10, countDigit(inputNumber) - exponent - 1);
      integerPart = inputNumber/baseNumber;
      fractionalPart = inputNumber - integerPart * baseNumber;
    }

  }

  /////SET MANTISSA BITS
  int8_t integerPartWidth = 53;
  while(!(integerPart & 0x10000000000000) && integerPartWidth > 0) {
    integerPart = integerPart << 1;
    integerPartWidth--;
  }
  
  
  uint64_t finalFractionalPart = 0;
  uint8_t fractionalPtr = 53 - integerPartWidth;
  while(fractionalPart != 0 && fractionalPtr > 0) {
    fractionalPart *= 2;
    uint8_t bit = 0;
    if(fractionalPart >= baseNumber) {
      fractionalPart-=baseNumber;
      bit = 1;
      isFirstDigitSet = true;
    }
    finalFractionalPart += (uint64_t)bit << (fractionalPtr-1);
    if(isFirstDigitSet) {
      fractionalPtr--;
    } else {
      integerPartWidth--;
    }
  }

  mantissa = (integerPart + finalFractionalPart) & 0xFFFFFFFFFFFFF;

  /////SET EXPONENT BITS
  exponent = (integerPartWidth - 1 + 1023);

}

uint8_t Float64::getSign() {
  return sign;
}

uint16_t Float64::getExponent() {
  return exponent;
}

uint64_t Float64::getMantissa() {
  return mantissa;
}

uint64_t Float64::getHexVersion() {
  
  return ((uint64_t)sign << 63) + ((uint64_t)(exponent & 0x7FF) << 52) + mantissa;
}


Float64 Float64::add(Float64& lOperand, Float64& rOperand) {

  Float64* rFloat;
  Float64* lFloat;

  if(this->getExponent() >= rOperand.getExponent()) {
    lFloat = &lOperand;
    rFloat = &rOperand;
  } else {
    rFloat = &lOperand;
    lFloat = &rOperand;
  }

  int16_t lExponent = lFloat->getExponent()-1023;
  int16_t rExponent = rFloat->getExponent()-1023;

  uint64_t lMantissa = lFloat->getMantissa() + ((uint64_t)1<<52);
  uint64_t rMantissa = (rFloat->getMantissa() + ((uint64_t)1<<52)) >> (lExponent - rExponent);

  uint8_t lSign = lFloat->getSign();
  uint8_t rSign = rFloat->getSign();

  uint64_t resultMantissa;
  uint8_t resultSign = 0;

  if(lSign == rSign) {
    resultMantissa = lMantissa + rMantissa;
    resultSign = lSign;
  } else {
     resultMantissa = (lMantissa > rMantissa) ? (lMantissa - rMantissa) : (rMantissa - lMantissa);
     resultSign = (lMantissa > rMantissa) ? lSign : rSign;
  }

  uint64_t bit = 0;
  uint8_t counter = 64;
  while(!bit) {
    counter--;
    bit = resultMantissa & ((uint64_t)1 << counter);
  }

  uint16_t resultExponent = lExponent + (counter - 52) + 1023;
  
  if(counter > 52) {
    resultMantissa = resultMantissa >> (counter - 52);
  } else {
    resultMantissa = resultMantissa << (52 - counter);
  }

  resultMantissa = resultMantissa & 0xFFFFFFFFFFFFF;

  return Float64{resultSign, resultExponent, (uint64_t)resultMantissa};
}

Float64 Float64::operator+(Float64& rOperand) {
  return add(*this, rOperand);
}

Float64 Float64::operator-(Float64& rOperand) {
  Float64 f{rOperand};
  if(f.sign == 1) {
    f.sign = 0;
  } else {
    f.sign = 1;
  }

  return add(*this, f);
}
