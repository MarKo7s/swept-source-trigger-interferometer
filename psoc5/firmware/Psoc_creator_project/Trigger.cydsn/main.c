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
int time = 0; // Sweep time duration in 'us'
float frequency = 0; // Frequency of the trigger output signal in 'Hz'
uint total_count = 0; // Count of trigger events 

//ModeFlags:
int operationMode = 0; // Normal  (0) --> Returns nothing after end of sweeps
                       // Time    (1) --> Returns time in um after each sweep event
                       // Count   (2) --> Returns number of trigger events after each sweep event
                       // frequecy(3) --> Returns frequency in Hz after each sweep event
                       // All     (4) --> Returns (1) (2) and (3) separeted by a space

//Commands:
int resultFreqMessage = 1;
int resultID = 1;
int resultFreq = 1;
int resultTotalCount = 1;
int resultSweepTime = 1;
int SetModeNormal = 1;
int SetModeTime = 1;
int SetModeCount = 1;
int SetModeFrequency = 1;
int SetModeAll = 1;
int GetMode = 1;

//Counter Interruption - Just for debugging
CY_ISR(INT_COUNTER){
    UART_PutString("Interrupt Triggered \n \r"); 
    state = count_out_Read();
    sprintf(str1,"%i\n\r",state);
    UART_PutString(str1);  
}

//Interruption generated after each completled laser sweep
CY_ISR(INT_SW){
    //Calculate the elapsed time during the sweep:
    last_time_count = time_count;
    time_current_counter = Timer_ReadCounter();
    time = (1 * last_time_count) +  (time_current_counter*1/24);
    //sprintf(str1,"%i\n\r",time);
    //UART_PutString(str1);
    //Calculate the frequency of the output signal:
    total_count = freqcounter_ReadCounter();
    frequency = ( total_count ) / ((float)time * 0.000001);
    //sprintf(str1,"%f\n\r",frequency);
    //UART_PutString(str1);
    
    //Generate a signal through COM port if set
    if (operationMode != 0){
        //Decide what output to provide
        switch(operationMode)
        {
            case 1: //Time
                sprintf(str1,"%i\n\r",time);
                break;
            case 2://Count
                sprintf(str1,"%i\n\r",total_count);
                break;
            case 3://Frequency
                sprintf(str1,"%f\n\r",frequency);
                break;
            case 4://All
                sprintf(str1,"%i %i %f\n\r",time, total_count, frequency);
                break;
        }
        UART_PutString(str1);
        
    }
    
    //Restart everything:
    freqcounter_WriteCounter(0);
    time_count = 0;        
}


//Interruption generated at the begining of each laser sweep
CY_ISR(INT_SWSTART){
}

//Interruption generated when timer counter overflows
CY_ISR(INT_TIMER){
    time_count = time_count + 1;
}

int main(void)
{   
    CyGlobalIntEnable; /* Enable global interrupts. */
    
    Comp_Start(); //Comparator Start  
    Counter_Start();// Counter Start
    //PWM_Start();
    Timer_Start();
    freqcounter_Start();

    
    //Debugging Interruptions
    
    /* //Disable Interruptions
    isr_count_Start();
    isr_count_StartEx(INT_COUNTER);
    */
    
    //Interruption set up
    isr_timer_Start();
    isr_timer_StartEx(INT_TIMER);
    
    isr_sw_Start();
    isr_sw_StartEx(INT_SW);
    
    //I do not need it now
    //isr_swStart_Start();
    //isr_swStart_StartEx(INT_SWSTART);
    
    //Serial communication by UART
    UART_Start();
    UART_PutChar(12);
    UART_PutString("Trigger assambled \n\r");
    
    //Set Counter (default configuration)
    count = 5; //Input Signal fre / count = Output signal freq
    Counter_WritePeriod(count);
    uint dutyCycle = 2;
    Counter_WriteCompare( (count / dutyCycle) - 1); //Dividing by two = 50% duty cycle of the output signal
    
    for(;;)
    {   
        if (UART_GetRxBufferSize()!=0 && sw_Read()==1){    
            rx = UART_GetChar();
            if (rx == '\n'){
                //////////COMMANDS//////////////
                sscanf(str,"%i",&new_count);
                resultID =  strcmp(str, "\r*IDN?");
                resultFreqMessage = strcmp(str, "\rfreq"); //check for freq pull
                resultFreq =  strcmp(str, "\rfreq_int");
                resultTotalCount = strcmp(str, "\rtrig_frames");
                resultSweepTime = strcmp(str, "\rsweep_time");
                SetModeNormal =  strcmp(str, "\rSetMode_normal");
                SetModeTime = strcmp(str, "\rSetMode_time");
                SetModeCount = strcmp(str, "\rSetMode_count");
                SetModeFrequency = strcmp(str, "\rSetMode_freq");
                SetModeAll = strcmp(str, "\rSetMode_all");
                GetMode = strcmp(str, "\rGetMode");
     
                ////////////////////////////////
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
                else if(resultID == 0){  
                    UART_PutString("PSOC 5LP Trigger; LastVersion:20/09/2022 \n\r");
                }else if(resultFreq == 0){
                    sprintf(str,"%f\n\r",frequency);         
                    UART_PutString(str);
                }else if(resultFreqMessage == 0){
                    sprintf(str,"Frequency of the trigger signal: %f Hz \n\r",frequency);         
                    UART_PutString(str);
                }else if(resultTotalCount == 0){
                    sprintf(str,"%d\n\r",total_count);         
                    UART_PutString(str);
                }else if(resultSweepTime == 0){
                    sprintf(str,"%d\n\r",time);         
                    UART_PutString(str);
                }
                //Operation Modes:
                else if(SetModeNormal == 0){
                    operationMode = 0;
                }else if(SetModeTime == 0){
                    operationMode = 1;
                }else if(SetModeCount == 0){
                    operationMode = 2;
                }else if(SetModeFrequency == 0){
                    operationMode = 3;
                }else if(SetModeAll == 0){
                    operationMode = 4;
                }else if(GetMode == 0){
                    sprintf(str,"%i\n\r",operationMode);         
                    UART_PutString(str);
                }else{  
                    //UART_PutString(str);
                        UART_PutString("NOT SUPPORTED COMMAND\r\n");         
                    }
                memset(str,0,sizeof(str));//clean string
                state = 0;
                new_count = 0;
            }else{
                str[state] = rx;
                state = state+1;
            }
        }
        if (UART_GetRxBufferSize()!= 0 && sw_Read() == 0){
            rx = UART_GetChar();
            if (rx == '\n'){ 
                UART_PutString("Wait until the laser sweep ends\r\n");               
            }
            UART_ClearRxBuffer();//Clear buffer to avoid problems
        }
       
    }
}

/* [] END OF FILE */
