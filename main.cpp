/* Karplus-Strong String Synthesis 
 * Language:  C
 * Author:  Cole Jepson
 * Date:  7/22/2017
 * References:  https://en.wikipedia.org/wiki/Xorshift, 
 * http://cmc.music.columbia.edu/MusicAndComputers/chapter4/04_09.php,
 * https://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis
 */

#include "mbed.h"

AnalogIn ADC16(p16);
AnalogOut DAC16(p18);
InterruptIn button(p21);

// White noise macro
#define WhiteL 512  // white noise array length

uint16_t WhiteOut[WhiteL] = {0};// WhiteOut is a buffer that holds white noise
volatile int reset = 1;         // reset is used as a flag to tell the 
                                // white noise generator to regenerate its 
                                // values after every button press

void button_press()
{
    int i,j;                    // declare counters
    int num_feedback = 600;     // amount of times the noise is fed back in LI
                                // into averaging loop
    float sdelay = 0.00001;     // wait time between each output sample
    
    for(j = 0; j < num_feedback; j++)
    {
        for(i = 1; i <= WhiteL; i++)
        {   // sum current sample with previous sample and divide by two
            WhiteOut[i] = (WhiteOut[i]+WhiteOut[i-1]) >> 1;
            DAC16.write_u16(WhiteOut[i]);
            wait(sdelay);
        }  
    }  
    reset = 1;      // assert flag so that new white noise values are generated
}

int main() 
{
    button.mode(PullUp);      // set Pin 21 mode to pull up
    uint32_t  randn, mask = 0;// declare 32-bit variables
    int i;                    // declare counter
                                    
    mask = (1 << 16) - 1;       // 16-bit mask to reduce 32-bit to 16-bit
    
    button.fall(&button_press); // every time the button is pressed, the voltage
                                // "falls" to ground which will trigger this
                                // interrupt to go to the button_press function
    while(1)
    {
        if(reset == 1)          // check to see if new white noise values
        {                       // are needed
            randn = ADC16.read_u16();   // read floating pin voltage on DIP16
            for(i = 0; i < WhiteL; i++)
            {
                randn ^= randn << 13;   // bit-shift to create random numbers
                randn ^= randn >> 17;
                randn ^= randn << 5;  
                WhiteOut[i] = randn & mask; // convert 32-bit uint to 16-bit  
            }
            reset = 0;                  // wait until another button is pressed
        }                               // to generate new white noise values
    }
}
