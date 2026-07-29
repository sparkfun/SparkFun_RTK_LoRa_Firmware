/*
 * A sketch which converts SPI clock and data to USB Serial on Teensy
 * 
 * Written by: P.C. for SparkFun Electronics
 * July 29th 2026
 * 
 * Licence: MIT
 * 
 * The data pin is sampled on the rising edge of the clock pin using standard
 * interrupts and digitalReadFast. We are not using actual SPI hardware due to
 * the lack of CS and problems resyncing the bit / byte alignment.
 * 
 * Set Optimize to Fastest
 * 
 * With the SPI clock running at 750 KBits/s:
 * Teensy 3.2 can just about keep up at 96MHz (overclock). Use 120MHz (overclock) for best results.
 * Teensy 4.0 can keep up at 120MHz. Use e.g. 240MHz for best results.
 */

const uint8_t pinClock = 0;
const uint8_t pinData = 1;

unsigned long previousMillis;
const unsigned long timeout = 1;

const uint16_t bufferSize = 1000;
const uint16_t bufferContingency = 100;
uint8_t bufferBytes[bufferSize + bufferContingency];
uint8_t *bufferPtr;

volatile uint8_t currentBit;
uint8_t newBits[8]; // Can't be volatile - for memcpy
uint8_t newBitsCopy[8];
volatile bool newByteSeen;

void isr()
{
  newBits[currentBit++] = digitalReadFast(pinData); // Read and store the data pin - fast!
  if (currentBit == 8) // Every 8th bit
  {
    memcpy(newBitsCopy, newBits, 8); // Copy the bits
    newByteSeen = true; // Set the flag
    currentBit = 0; // Start over
  }
  //asm("DSB");
}

void setup() {
  pinMode(pinClock, INPUT_PULLDOWN);
  pinMode(pinData, INPUT_PULLDOWN);
  
  Serial.begin(0); // Teensy USB Serial
  while (!Serial);
  Serial.println("Teensy SPI to USB Serial");

  currentBit = 0;
  newByteSeen = false;
  bufferPtr = &bufferBytes[0];

  // Interrupt on the rising edge of the clock
  attachInterrupt(digitalPinToInterrupt(pinClock), isr, RISING);

  // Set the interrupt priority
  #if defined(__MK20DX256__)
  NVIC_SET_PRIORITY(IRQ_PORTB, 0); // On Teensy 3.2, GPIO 0 and 1 are on Port B
  #elif defined(__IMXRT1062__) && defined(ARDUINO_TEENSY40)
  NVIC_SET_PRIORITY(IRQ_GPIO6789,0); // On Teensy 4.0, all the pin interrupts use IRQ_GPIO6789
  #endif
  
  previousMillis = millis();
}

void loop() {
  if (newByteSeen) // Have we received 8 bits?
  {
    newByteSeen = false; // Clear the flag

    *bufferPtr = 0; // Clear this byte
    const uint8_t bitPos[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 }; // MSB first
    for (uint8_t i = 0; i < 8; i ++)
      if (newBitsCopy[i]) // Copy any set bits into the buffer
        *bufferPtr |= bitPos[i]; // MSB first

    previousMillis = millis(); // Reset the timer

    bufferPtr++; // Increment the buffer pointer
  }
  // else check for a timeout
  else if ((millis() - previousMillis) > timeout)
  {
    currentBit = 0; // Reset currentBit on timeout - reset the bit / byte alignment
    previousMillis = millis(); // Reset the timer

    if (bufferPtr > &bufferBytes[0]) // Anything to print?
    {
      //Serial.println((const char *)&bufferBytes[0]);
      Serial.write(bufferBytes, bufferPtr - &bufferBytes[0]); // Print the buffer
      //Serial.println();
      Serial.send_now();
      bufferPtr = &bufferBytes[0]; // Reset the buffer pointer
    }
  }
  // else check for buffer full
  else if (bufferPtr >= (&bufferBytes[0] + bufferSize))
  {
    Serial.println("<<< BUFFER FULL >>>");
    //Serial.println((const char *)&bufferBytes[0]);
    Serial.write(bufferBytes, bufferPtr - &bufferBytes[0]); // Print the buffer
    //Serial.println();
    Serial.send_now();
    bufferPtr = &bufferBytes[0]; // Reset the buffer pointer
  }
}
