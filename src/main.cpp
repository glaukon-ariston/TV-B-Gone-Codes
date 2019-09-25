// From the project at https://github.com/shirriff/Arduino-TV-B-Gone

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "WORLD_IR_CODES.h"

/* This is kind of a strange but very useful helper function
 Because we are using compression, we index to the timer table
 not with a full 8-bit byte (which is wasteful) but 2 or 3 bits.
 Once code_ptr is set up to point to the right part of memory,
 this function will let us read 'count' bits at a time which
 it does by reading a byte into 'bits_r' and then buffering it. */

uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
uint8_t const *code_ptr;

// we cant read more than 8 bits at a time so dont try!
uint8_t read_bits(uint8_t count)
{
  uint8_t i;
  uint8_t tmp=0;

  // we need to read back count bytes
  for (i=0; i<count; i++) {
    // check if the 8-bit buffer we have has run out
    if (bitsleft_r == 0) {
      // in which case we read a new byte in
      bits_r = *code_ptr++;
      // and reset the buffer size (8 bites in a byte)
      bitsleft_r = 8;
    }
    // remove one bit
    bitsleft_r--;
    // and shift it off of the end of 'bits_r'
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count-1-i));
  }
  // return the selected bits in the LSB part of tmp
  return tmp;
}



/*
The C compiler creates code that will transfer all constants into RAM when
 the microcontroller resets.  Since this firmware has a table (powerCodes)
 that is too large to transfer into RAM, the C compiler needs to be told to
 keep it in program memory space.  This is accomplished by the macro PROGMEM
 (this is used in the definition for powerCodes).  Since the C compiler assumes
 that constants are in RAM, rather than in program memory, when accessing
 powerCodes, we need to use the pgm_read_word() and pgm_read_byte macros, and
 we need to use powerCodes as an address.  This is done with PGM_P, defined
 below.
 For example, when we start a new powerCode, we first point to it with the
 following statement:
 PGM_P thecode_p = pgm_read_word(powerCodes+i);
 The next read from the powerCode is a byte that indicates the carrier
 frequency, read as follows:
 const uint8_t freq = pgm_read_byte(code_ptr++);
 After that is a byte that tells us how many 'onTime/offTime' pairs we have:
 const uint8_t numpairs = pgm_read_byte(code_ptr++);
 The next byte tells us the compression method. Since we are going to use a
 timing table to keep track of how to pulse the LED, and the tables are
 pretty short (usually only 4-8 entries), we can index into the table with only
 2 to 4 bits. Once we know the bit-packing-size we can decode the pairs
 const uint8_t bitcompression = pgm_read_byte(code_ptr++);
 Subsequent reads from the powerCode are n bits (same as the packing size)
 that index into another table in ROM that actually stores the on/off times
 const PGM_P time_ptr = (PGM_P)pgm_read_word(code_ptr);
 */


/*
Export the IR codes in the format required by the TVKILL Android app, see

\infrared\TVKILL\app\src\main\java\com\redirectapps\tvkill\BrandContainer.kt

    private val sony = Brand(
            "sony",
            arrayOf(
                    //Sony[0](40064,1,1,96,24,48,24,48,24,48,24,48,24,24,24,48,24,24,24,48,24,24,24,24,24,24,24,24,985,96,24,48,24,48,24,48,24,48,24,24,24,48,24,24,24,48,24,24,24,24,24,24,24,24,985,96,24,48,24,48,24,48,24,48,24,24,24,48,24,24,24,48,24,24,24,24,24,24,24,24,5128)
                    Pattern(40064, intArrayOf(1, 1, 96, 24, 48, 24, 48, 24, 48, 24, 48, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 985, 96, 24, 48, 24, 48, 24, 48, 24, 48, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 985, 96, 24, 48, 24, 48, 24, 48, 24, 48, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 5128))),
            //Mute-pattern
            Pattern(40192, intArrayOf(1, 1, 96, 24, 24, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 1060, 96, 24, 24, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 1060, 96, 24, 24, 24, 24, 24, 48, 24, 24, 24, 48, 24, 24, 24, 24, 24, 48, 24, 24, 24, 24, 24, 24, 24, 24, 5144))
    )

According to https://github.com/42SK/TVKILL/wiki/How-to-add-IR-patterns-to-TV-KILL

The constructor of the Pattern class requires two parameters:
  1. The pattern's frequency in Hertz
  2. The alternating on/off pattern in periods of the carrier frequency


// However some codes dont use PWM in which case we just turn the IR (freq=0)
// LED on for the period of time.
digitalWrite(IRLED, HIGH);

*/

void sendAllCodes(const char* groupName, uint8_t num_codes, const IrCode* const data_base_ptr[])
{
  printf("    /*\n\
    Conversion program at https://github.com/glaukon-ariston/TV-B-Gone-Codes\n\
    IR database from https://github.com/shirriff/Arduino-TV-B-Gone\n\
    According to https://github.com/42SK/TVKILL/wiki/How-to-add-IR-patterns-to-TV-KILL\n\
    The constructor of the Pattern class requires two parameters:\n\
      1. The pattern's frequency in Hertz\n\
      2. The alternating on/off pattern in periods of the carrier frequency\n\
    */\n");
  printf("    private val %s = Brand(\n        \"%s\",\n        arrayOf(\n", groupName, groupName);
  // for every POWER code in our collection
  for (uint8_t i=0 ; i<num_codes; i++) 
  {
    const IrCode* irCode = data_base_ptr[i];
    code_ptr = irCode->codes;
    if(irCode->timer_val == 0) continue;    // some codes dont use PWM (freq==0), ignore them.

    if(i > 0) {
      printf(",\n");
    }

    // Transmit all codeElements for this POWER code
    // (a codeElement is an onTime and an offTime)
    // transmitting onTime means pulsing the IR emitters at the carrier
    // frequency for the length of time specified in onTime
    // transmitting offTime means no output from the IR emitters for the
    // length of time specified in offTime

    double period = 1000000. / irCode->timer_val;     // in microseconds
    printf("            Pattern(%d /*T=%fus*/, intArrayOf(", irCode->timer_val, period);
    for (uint8_t k=0; k < irCode->numpairs; k++) {
      uint16_t ti;
      uint16_t ontime, offtime;

      // Read the next 'n' bits as indicated by the compression variable
      // The multiply by 4 because there are 2 timing numbers per pair
      // and each timing number is one word long, so 4 bytes total!
      ti = read_bits(irCode->bitcompression) * 2;

      // read the onTime and offTime from the program memory
      // unit is 10s of microseconds
      ontime = irCode->times[ti];
      offtime = irCode->times[ti+1];
      // transmit this codeElement (ontime and offtime)
      if(k > 0) {
        printf(",");
      }
      //printf("%d,%d", ontime*10, offtime*10);   // in microseconds
      printf("%ld,%ld", lround(ontime*10/period), lround(offtime*10/period));   // in periods
    }
    printf("))");

    //Flush remaining bits, so that next code starts
    //with a fresh set of 8 bits.
    bitsleft_r=0;

  } //end of POWER code for loop
  printf("\n        ),\n        //Mute-pattern\n        Pattern(0, intArrayOf(0,0))\n    )\n");
} //end of sendAllCodes



int main(int argc, char* argv[]) {
  sendAllCodes("tvBgoneNA", num_NAcodes, NApowerCodes);
  printf("\n");
  sendAllCodes("tvBgoneEU", num_EUcodes, EUpowerCodes);
  printf("\n    val allBrands = arrayOf(samsung, sony, lg, panasonic, philips, nec, sharp, jvc, toshiba, mitsubishi, vizio, rca, pioneer, hisense, akai, aoc, tvBgoneEU, tvBgoneNA)\n");
  printf("\n");
  return 0; // Indicates that everything went well.  
}