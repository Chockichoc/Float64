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

  return Float64{resultSign, resultExponent, resultMantissa};
}

Float64 Float64::operator+(Float64& rOperand) {
  return add(*this, rOperand);
}

Float64 Float64::operator-(Float64 rOperand) {
  if(rOperand.sign == 1) {
    rOperand.sign = 0;
  } else {
    rOperand.sign = 1;
  }

  return add(*this, rOperand);
}

Float64 Float64::operator*(Float64& rOperand) {
  uint8_t resultSign = this->getSign() ^ rOperand.getSign();

  uint64_t lMantissa = this->getMantissa() + ((uint64_t)1<<52);
  uint64_t rMantissa = rOperand.getMantissa() + ((uint64_t)1<<52);
  
  uint64_t MSBResultMantissa = 0, LSBResultMantissa = 0;
  for(uint8_t i = 0; i < 64; i++) {
    
    uint64_t temp_LSBResultMantissa = LSBResultMantissa + ((rMantissa & ((uint64_t)1 << i)) ? (lMantissa << i) : 0);
    if(temp_LSBResultMantissa < LSBResultMantissa) {
      MSBResultMantissa++;
    }
    
    LSBResultMantissa = temp_LSBResultMantissa;
    MSBResultMantissa += (rMantissa & ((uint64_t)1 << i)) ? (lMantissa >> (64 - i)) : 0;
    
  }

  uint64_t bit = 0;
  uint8_t counter = 0;
  while(!bit) {
    counter++;
    bit = MSBResultMantissa & ((uint64_t)1 << (64-counter));
  }

  uint8_t offset = (128-(2*52)) - counter;
  uint64_t resultMantissa = ((MSBResultMantissa << (12 - offset)) + (LSBResultMantissa >> (52 + offset))) & 0xFFFFFFFFFFFFF;
  uint16_t resultExponent = this->getExponent()+ rOperand.getExponent() + offset - 1023;
  
  return Float64{resultSign, resultExponent, resultMantissa};
}

Float64 Float64::operator/(Float64& rOperand) {
  uint8_t resultSign = this->getSign() ^ rOperand.getSign();
  
  uint64_t resultMantissa = 0;
  uint64_t dividendMantissa = this->getMantissa() + ((uint64_t)1<<52);
  uint64_t dividerMantissa = rOperand.getMantissa() + ((uint64_t)1<<52);
  uint8_t offset = dividerMantissa > dividendMantissa ? 1 : 0;
  
  for(uint8_t i = 0; i < 53; i++) {
    if(dividendMantissa >= dividerMantissa) {
      dividendMantissa -= dividerMantissa;
      resultMantissa += ((uint64_t)1<<(63-i));
    } 
    dividendMantissa = dividendMantissa<<1;
  }

    Serial.println("Mantissa");
  Serial.println((uint32_t)(resultMantissa>>32), HEX);
  Serial.println((uint32_t)resultMantissa, HEX);

  uint64_t bit = 0;
  uint8_t counter = 64;
  while(!bit && counter != 0) {
    counter--;
    bit = resultMantissa & ((uint64_t)1 << counter);
  }

  if(counter > 52) {
    resultMantissa = (resultMantissa >> (counter - 52)) & 0xFFFFFFFFFFFFF;
  } else {
    resultMantissa = (resultMantissa << (52 - counter)) & 0xFFFFFFFFFFFFF;
  }

  Serial.println("Mantissa");
  Serial.println((uint32_t)(resultMantissa>>32), HEX);
  Serial.println((uint32_t)resultMantissa, HEX);
  
  uint16_t resultExponent = this->getExponent() - rOperand.getExponent() - 1023 - 2 - offset;

  return Float64{resultSign, resultExponent, resultMantissa};
}
