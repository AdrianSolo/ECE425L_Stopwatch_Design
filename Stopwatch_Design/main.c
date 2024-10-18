/*
 * @file main.c
 *
 * @brief Main source code for the Stopwatch_Design program.
 *
 * This file contains the main entry point and function definitions for the Stopwatch_Design program.
 * This lab involves designing a basic stopwatch. It demonstrates edge-triggered interrupts, 
 * and it interfaces with the following:
 *  - User LED (RGB) Tiva C Series TM4C123G LaunchPad
 *	- EduBase Board LEDs (LED0 - LED3)
 *	- EduBase Board Push Buttons (SW2 - SW3)
 *	- EduBase Board Seven-Segment Display
 *	- PMOD BTN module
 *
 * It configures the pins used by the EduBase Board push buttons (SW2 and SW3) and the PMOD
 * BTN module to generate interrupts on rising edges.
 * 
 * Timer 0A is used to generate periodic interrupts every 1 millisecond. The values of the 
 * stopwatch (milliseconds, seconds, and minutes) will increment in the Timer 0A
 * periodic task. The PMOD BTN module will be used to control the stopwatch.
 *
 * @author
 */

#include "TM4C123GH6PM.h"
#include "GPIO.h"
#include "PMOD_BTN_Interrupt.h"
#include "EduBase_Button_Interrupt.h"
#include "Seven_Segment_Display.h"
#include "Timer_0A_Interrupt.h"

// Declare the user defined function prototype for PMOD_BTN_Interrupt
void PMOD_BTN_Handler(uint8_t pmod_btn_status);

// Declare the user defined function prototype for EduBase_Button_Interrupt
void EduBase_Button_Handler(uint8_t edubase_button_status);

// Initialize a global variable for an 8-bit counter
static uint8_t counter = 0;

// Declare the function Prototype for the function that calculates 
// stopwatch value and stores it in an array
void Calculate_Stopwatch_Value(uint8_t stopwatch_value[]);

// Declare the function prototype for the user-defined function for Timer 0A
void Timer_0A_Periodic_Task(void);

// initialize a globar variable for timer 0A to keep track of the elapsed time in milliseconds
static uint8_t ms_elapsed = 0;

// Initialize global variables to keep track of the stopwatch value (o.e milliseconds, seconds, and minutes)
// "milliseconds" is updated every 100 ms (Range: 0 to 999 ms)
// "seconds" is updated every 1000 ms (Range: 0 to 59 s)
// "minutes" is updated every 60 seconds (Range: 0 to 9 minutes)
static uint8_t milliseconds = 0;
static uint8_t seconds = 0;
static uint8_t minutes = 0;

// Initialize global flags for starting and resetting the stopwatch
static uint8_t start_stopwatch = 0;
static uint8_t reset_stopwatch = 0;

int main(void)
{
	// Initialize the push buttons on the PMOD BTN module (Port A)
	PMOD_BTN_Interrupt_Init(&PMOD_BTN_Handler);
	
	// Initialize the LEDs on the EduBase Board (Port B)
	EduBase_LEDs_Init();
	
	// Initialize the SysTick timer used to provide blocking delay functions
	SysTick_Delay_Init();
	
	// Initialize the Seven Segment Display (Port B and C)
	Seven_Segment_Display_Init();
	
	// Initialize the SW2 and SW3 on the EduBase board with interrupts enabled (Port D)
	EduBase_Button_Interrupt_Init(&EduBase_Button_Handler);
	
	// Initialize the RGB LED (Port F)
	RGB_LED_Init();
	
	// Initialize the timer 0A to generate periodic interrupts every 1ms
	Timer_0A_Interrupt_Init(&Timer_0A_Periodic_Task);
	
	// Initialize a uint8_t array to store each digit of the stopwatch value
	uint8_t stopwatch_value[4] = {0};
	
	while(1)
	{
		Calculate_Stopwatch_Value(stopwatch_value);
		Seven_Segment_Display_Stopwatch(stopwatch_value);
	}
}






/**
 * @brief
 *
 * @param
 *
 * @return
 */
void PMOD_BTN_Handler(uint8_t pmod_btn_status)
{
	switch(pmod_btn_status)
	{
		// BTN0 (PA2) is pressed
		case 0x04:
		{
			RGB_LED_Output(RGB_LED_GREEN);
			start_stopwatch = 0x01;
			break;
		}
		
		// BTN1 (PA3) is pressed
		case 0x08:
		{
			RGB_LED_Output(RGB_LED_RED);
			start_stopwatch = 0x00;
			break;
		}
		
		// BTN2 (PA4) is pressed
		case 0x10:
		{
			RGB_LED_Output(RGB_LED_OFF);
			reset_stopwatch = 0x01;
			break;
		}
		
		// BTN3 (PA5) is pressed
		case 0x20:
		{
			
			break;
		}
		
		default:
		{
			break;
		}
	}
}






/**
 * @brief
 *
 * @param
 *
 * @return
 */
void EduBase_Button_Handler(uint8_t edubase_button_status)
{
	switch (edubase_button_status)
	{
		case 0x08:
		{
			if (counter >= 15)
			{
					counter = 0;
				}
				else
					{
					counter = counter + 1;
				}
				break;
			}
			
		case 0x04:
		{
			if (counter <= 0)
			{
				counter = 15;
			}
			else
			{
				counter = counter - 1;
			}
			break;
		}
		
		default:
		{
			break;
		}
	}
}
/**
 * @brief
 *
 * @param uint8_t stopwatch_value[]
 *
 * @return None
 */
void Calculate_Stopwatch_Value(uint8_t stopwatch_value[]) 
{
	// Store the "milliseconds" value in the first index of the array
	stopwatch_value[0] = milliseconds;
	
	// store in the least significant digit of the "seconds" value
	// in the second index of the array
	stopwatch_value[1] = seconds % 10;
	
	// store the most significant digit of the "seconds" value
	// in the third index of the array
	stopwatch_value[2] = seconds / 10;
	
	// store the "minutes" value in the fourth index of the array
	stopwatch_value[3] = minutes;
}

/**
 * @brief This function will manage the periodic tasks for the stopwatch, such as 
 * incrementing time variables and handling start and reset functions.
 * 
 * The function checks two flags: `start_stopwatch` and `reset_stopwatch`. 
 * If `start_stopwatch` is selected, it increments the stopwatch time in milliseconds, 
 * seconds, and minutes. If `reset_stopwatch` is set, it clears all 
 * variables and resets the timer for the stopwatch.
 * 
 * @param None
 * 
 * @return None
 */
  
void Timer_0A_Periodic_Task(void)
{
    // Check if the start_stopwatch flag is set
    if (start_stopwatch == 0x01)
    {
        ms_elapsed++;
        
        // Check if ms_elapsed has reached 100 milliseconds
        // debug issue: The milliseconds variable did not reset to 0 after reaching 10.
        // debug solution: added a reset to the function for if the milliseconds pass 10. 
        if (ms_elapsed > 99)
        {
            ms_elapsed = 0; // Reset ms_elapsed to 0 after 100 milliseconds
            milliseconds++; // Increment milliseconds
            
            
            if (milliseconds > 9) // will reset milliseconds as well as increment seconds timer 
            {
                milliseconds = 0; // resets the milliseconds timer
                seconds++;
                
              
                if (seconds > 59) // resets seconds as well as increments by one every minute that passes
                {
                    seconds = 0; // reset seconds every 60 seconds
                    minutes++; // increment minute timer
                    
                    if (minutes > 9) // will reset minutes after hitting ten to prevent overflow
                    {
                        minutes = 0; // resets the minutes counter
                    }
                }
            }
        }
    }
    
    // checks to see if reset stopwatch has been selected as a flag
    if (reset_stopwatch == 0x01)
    {
        reset_stopwatch = 0x00; // Clear reset flag after reset
        start_stopwatch = 0x00; // Stop the stopwatch after reset
        ms_elapsed = 0;         // Reset ms_elapsed to 0
        milliseconds = 0;       // Reset milliseconds to 0
        seconds = 0;            // Reset seconds to 0
        minutes = 0;            // Reset minutes to 0
    }
}
