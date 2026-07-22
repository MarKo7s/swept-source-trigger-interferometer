/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

uint count = 0;
char str[30] = "\0";
char str1[30];
int counter=0;
int state = 0;
uint new_count = 0;
int comp = 0;
uint16 buff_size = 0;
uint8 rx = 0;

int time_count = 0;
int last_time_count = 0;
int time_current_counter = 0;
int time = 0;
float frequency = 0;

int result = 1;
int resultID = 1;
int resultFreq = 1;

CY_ISR(INT_COUNTER){
    UART_PutString("Interrupt Triggered \n \r"); 
    state = count_out_Read();
    sprintf(str1,"%i\n\r",state);
    UART_PutString(str1);  
}

CY_ISR(INT_SW){
    last_time_count = time_count;
    time_current_counter = Timer_ReadCounter();
    time = (1 * last_time_count) +  (time_current_counter*1/24);
    //sprintf(str1,"%i\n\r",time);
    //UART_PutString(str1);
    
    frequency = (freqcounter_ReadCounter() ) / ((float)time * 0.000001);
    //sprintf(str1,"%f\n\r",frequency);
    //UART_PutString(str1);
    
    freqcounter_WriteCounter(0);
    time_count = 0;        
}

CY_ISR(INT_TIMER){
    time_count = time_count + 1;
}

int main(void)
{   
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    Comp_Start(); //Comparator Start  
    Counter_Start();// Counter Start
    PWM_Start();
    Timer_Start();
    freqcounter_Start();

    
    //Debugging Interruptions
    
    /* //Disable Interruptions
    isr_count_Start();
    isr_count_StartEx(INT_COUNTER);
    
    isr_sw_Start();
    isr_sw_StartEx(INT_SW);
   */  
    isr_timer_Start();
    isr_timer_StartEx(INT_TIMER);
    
    isr_sw_Start();
    isr_sw_StartEx(INT_SW);
    
    //Serial communication by UART
    UART_Start();
    UART_PutChar(12);
    UART_PutString("Trigger assambled \n\r");
    
    //Set Counter (default configuration)
    count = 5; //Input Signal fre / count = Output signal freq
    Counter_WritePeriod(count);
    Counter_WriteCompare( (count/2) - 1); //Dividing by two = 50% duty cycle of the output signal
    
    for(;;)
    {   
        if (UART_GetRxBufferSize()!=0 && sw_Read()==1){    
            rx = UART_GetChar();
            if (rx == '\n'){
                sscanf(str,"%i",&new_count);
                result = strcmp(str, "\rfreq"); //check for freq pull
                resultID =  strcmp(str, "\r*IDN?");
                resultFreq =  strcmp(str, "\rfreq_int");
                if(new_count>0 && new_count<9999999){
                    sprintf(str,"Count changed to: %i\n\r",new_count);         
                    UART_PutString(str);
                    count = new_count; //Input Signal fre / count = Output signal freq
                    Counter_WritePeriod(count);
                    if (new_count == 1){
                        comp = 0;
                    }else{comp = new_count/2 - 1;
                    }
                    Counter_WriteCompare(comp); //Dividing by two = 50% duty cycle of the output signal
                    }
                else if(result == 0){
                    sprintf(str,"Frequency of the trigger signal: %f Hz \n\r",frequency);         
                    UART_PutString(str);
                } else if(resultID == 0){  
                    UART_PutString("PSOC 5LP Trigger; LastVersion:25/02/2020 \n\r");
                }else if(resultFreq == 0){
                    sprintf(str,"%f\n\r",frequency);         
                    UART_PutString(str);
                }else{  
                 
                    //UART_PutString(str);
                        UART_PutString("No valid value\r\n");         
                    }
                memset(str,0,sizeof(str));//clean string
                state = 0;
                new_count = 0;
            }else{
                str[state] = rx;
                state = state+1;
            }
        }
        if (UART_GetRxBufferSize()!=0 && sw_Read()==0){
            rx = UART_GetChar();
            if (rx == '\n'){ 
                UART_PutString("Wait until the laser sweep ends\r\n");               
            }
            UART_ClearRxBuffer();//Clear buffer to avoid problems
        }
       
    }
}

/* [] END OF FILE */
