#include "mbed.h"
InterruptIn button(USER_BUTTON);//Interrupt on user button

Timer debounce; // define timer object
// debounce is the local name of the timer

void is_pressed() {
  if (debounce.read_ms() > 2000) {  // only allow calling the function is_pressed() if the debounce time passed has passed 2 seconds. 
    
    printf("debounce the user button.\n\r");
    debounce.reset(); // Reset the timer to 0 when the function is performed.
  }
}

//DIP 4 switches
BusIn Nibble(D9, D8, D7, D6); // Change these pins to buttons on your board.
//DigitalOut led(D2); // toggle 

extern int * ReadDS1621(void);
PwmOut servo(D11);

// LDR 
AnalogIn ADC(A5); 

// 4RED LEDs
DigitalOut Led1(D10);
DigitalOut Led2(D11);
DigitalOut Led3(D12); 
DigitalOut Led4(D13);

// RGB LED 
DigitalOut led1(D2);
DigitalOut led2(D3); 
DigitalOut led3(D4);

// adxl335
AnalogIn analog_AccelX(A0); // output of x-axis at analog pin A0
AnalogIn analog_AccelY(A1); // output of y-axis at analog pin A1
AnalogIn analog_AccelZ(A2); // output of z-axis at analog pin A2



AnalogIn pot(A4);


// main() runs in its own thread in the OS
int main() {    
    
    int* temp;// define address to store the array
    
    float Vin_r;
    float Vin_v;
    
   
    while (true) 
    {
       
        debounce.start(); // Start the timer

        if(button == 0) 
        {
            // Button is pressed
           is_pressed();  // considering the user button is pressed to display the value after the debounce time passed has passed 2000 milli-seconds.
           
             // check bits set in nibble
            switch(Nibble & Nibble.mask()) { 
            // read the bus and mask out bits not being used
    
            case 0x01: 
            printf("0001,       D6  is high \n\r");
            temp = ReadDS1621(); //get the temperature data
            char deg = 0x00B0; /*UTF-16 Encoding for °  ...however this gives ==>  Warning: Transfer of control bypasses initialization "switch(nibble & nibble.mask()) {" */ 

           
            if(temp[0] >= 25)
            {
                printf("Temperature=%i.%i %cC \n\r",temp[0],temp[1],deg); // use format specifier %c
                //servo = 0 degree position;   // thermostat controller is ‘off’.
                servo.period_ms(20);
                servo.pulsewidth_us(1000);
            } 
            else if (temp[0]< 25) 
            {
                printf("Temperature=%i.%i %cC \n\r",temp[0],temp[1],deg); // use format specifier %c
                //servo = 135 degree position;  //  thermostat controller is ‘on’.
                servo.period_ms(20);
                servo.pulsewidth_us(2500);
            }
            
            break;
            case 0x02:
            printf("0010,       D7 is high \n\r");
            float brightness; 
            brightness= 1-ADC.read(); 
            float I_val; 
            I_val =brightness*100;//Intensity value= I_val where at 100%, all LEDs will be off.
            
            
             if(I_val == 0) 
            {
                printf("light Level =%3.1f%%\n\r", I_val);
                printf("Ambient light level = * * * * * \n\r");                
                Led1.write(1); //duty cycle as 20% of 1 period (1 second) 
                Led2.write(1); 
                Led3.write(1); 
                Led4.write(1); 
                break; 
            }   
            else if(I_val> 0 && I_val<25) 
            {
                printf("light Level =%3.1f%%\n\r", I_val);
                printf("Ambient light level = * * * * \n\r");              
                Led1.write(0); //duty cycle as 20% of 1 period (1 second) 
                Led2.write(1); 
                Led3.write(1); 
                Led4.write(1); 
                break; 
            }   
            else if(I_val >=25 && I_val <50)
            {
                printf("light Level =%3.1f%%\n", I_val);
                printf("Ambient light level = * * *\n\r");
                
                Led1.write(0);                
                Led2.write(0);                
                Led3.write(1);
                Led4.write(1); 
                  
                break; 
                } 
                else if(I_val >=50 && I_val <75)
                {
                    printf("light Level =%3.1f%%\n", I_val);
                    printf("Ambient light level = *  * \n\r");
                    
                    Led1.write(0);                    
                    Led2.write(0);                   
                    Led3.write(0);                  
                    Led4.write(1);                   
                     break; 
                } 
                else if (I_val >=75 && I_val<100)
                {
                    printf("light Level =%3.1f%%\n", I_val);
                    printf("Ambient light level = * \n\r");                 
                    Led1.write(0);                    
                    Led2.write(0);                   
                    Led3.write(0);                  
                    Led4.write(0);
                     
                      break; 
                } 
                else  
                {
                    printf("light Level =%3.1f%%\n", I_val);
                    wait(0.1);
                    printf("ERROR!"); 
                    break;
                }
                
               
                
                
            case 0x04: 
            printf("0100,       D8 is high \n\r");
            /* The code below uses the triple axis analogue accelerometer (adxl335) to detect fall and reperesent an alarm using RGB led. 
            Alarm is triggered when acceleration values fall above 2g or below -2g.  
            ------------ Using the formula: g = [(Analog.read()-0.5)* 6] to calculate the expected analogue value for acceleration at 2g and -2g.---------- */

            float x=analog_AccelX.read();     
            float y=analog_AccelY.read();
            float z =analog_AccelZ.read();
            float a=0.833; // where a is expected analog value for acceleration = 2g
            float X= (x-0.500)*6.000;// calculated acceleration based on read values from accelerometer
            float Y= (y-0.500)*6.000;
            float Z =(z-0.500)*6.000;
            float b=0.167;// where b is expected analog value for acceleration = -2g
            
            /*Using Or operation to set the conditions for to trigger the alarm on any of the axis*/
            if (x>a|| x<b|| y>a|| y<b|| z>a|| z<b) 
            {
                printf("It's a fall\t\t Output acceleration  X, Y, Z readings = %3.3f\t%3.3f\t%3.3f\t\n", X, Y,Z);
               
                wait(0.2);
                led1.write(1); //led1 is red      
                led2.write(0); //led2 is green
                led3.write(0); // led3 is blue
                wait(0.2);
                led1.write(0);        
                led2.write(1);
                led3.write(0);
                wait(0.2);
                led1.write(0);        
                led2.write(0);
                led3.write(1);
                wait(0.2);
                 break;
            } 
            else 
            {
                printf(" Output acceleration  X, Y, Z readings = %3.3f\t%3.3f\t%3.3f\t\n", X, Y,Z);
                // Values of acceleration of 3 directions 
                wait(0.2);
            }
            break; 
            
            case 0x8: 
            printf("1000,        D9 is high \n\r");
            Vin_r = pot.read(); // Read the analog input value (value from 0.0 to 1.0 = full ADC conversion range)
            Vin_v = Vin_r * 3300; // Converts value in the 0V-3.3V range
            // Display values
            printf("Output voltage value from the potential meter = %f = %.0f mV\n\r", Vin_r, Vin_v);
            /* Rotate the poteniometer to left to decrease the vale and to right to increase the output voltage value. 
            The output voltage values of the potentiometer ranges from 0.000000 (i.e., 0mV) 1.000000(i.e., 3300mV)*/
            break;
            
            
            default: printf("Invalid switch value. \n\r");
            break;
            
            }   
        }
        else
        {
            printf("button is not pressed.\n\r");
        }
        wait(0.5);
        
    }
}
