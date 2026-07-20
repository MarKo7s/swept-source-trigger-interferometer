import serial

#\r was added before every command to make it compatible with serial port protocol in c++/cli
feedbackmode = {'Normal':0, 'Time':1, 'Count':2, 'Frequency': 3, 'All':4}


class PSOC5_trigger:
    def __init__(self, COM, BR = 115200, timeout = 1): #COM port and BauRate=default 115200
        self.COM = COM
        self.BR = BR
        try:
            self.PSOC5_serial_COM = serial.Serial(self.COM, self.BR ,timeout = timeout )
        except serial.serialutil.SerialException:
            print(f'Connection to {COM} port failed')
            
        
        #Key is the command to gerate method in python
        #First iteam of the list is the PSOC command
        #Second item of the list is the mode
        #Possible Notes
        self.PSOCTriggerCommands = {'ID': ['*IDN?', 'R', ''],
                         'GetFrequencyMessage': ['freq', 'R', ''],
                         'GetFrequency': ['freq_int','R', 'in Hz'],
                         'GetTriggeredFrames': ['trig_frames','R', ''],
                         'GetSweepTime': ['sweep_time','R', 'in us'],
                         'SetModeNormal': ['SetMode_normal','W', ''],
                         'SetModeTime': ['SetMode_time','W', ''],
                         'SetModeCount': ['SetMode_count','W', ''],
                         'SetModeFrequeny': ['SetMode_freq', 'W', ''],
                         'SetModeAll': ['SetMode_all','W' , ''],
                         'GetMode': ['GetMode','R', feedbackmode]}
    
        self.GenerateMethodsFromDictionary()
    
    def discoverMethods(self):
        for idx, key in enumerate(self.PSOCTriggerCommands.keys()):
            print(f'{idx} - {key} - {self.PSOCTriggerCommands[key][2]}')
    
    def SetFreqDivision(self, count):
        encoded_count = self.encondeCommand(count)
        return(self.decodeMessage(self.GetSerial(encoded_count)))
            
    # Pure pyserial stuff:
    def GetSerial(self,cmd):
        self.PSOC5_serial_COM.write(cmd)
        return(self.PSOC5_serial_COM.readline())

    def SetSerial(self, cmd):
        self.PSOC5_serial_COM.write(cmd)
        
    def pullbuffer(self):
        return(self.PSOC5_serial_COM.readline())
    
    def waitForSignal(self):
        return(self.decodeMessage(self.PSOC5_serial_COM.readline()))
    
    def SetSerialTimeOut(self, t):
        self.PSOC5_serial_COM.timeout = t
    
    def GetSerialTimeOut(self):
        return self.PSOC5_serial_COM.timeout
    
    def flushSerialBuffer(self):
        self.PSOC5_serial_COM.reset_input_buffer()
        
    def CloseSerialConnection(self):
        self.PSOC5_serial_COM.close()
        print("Serial communication with PSOC closed")
        
    #Gerate PSOC TRIGGER methods
    
    @staticmethod
    def encondeCommand(cmd):
        command = (f'\r{cmd}\n').encode('utf-8')
        return(command)
    @staticmethod
    def decodeMessage(msg):
        message = msg.decode('utf-8').strip()
        return(message)
    
    def GenerateMethodsFromDictionary(self): 
        supportedCmd = self.PSOCTriggerCommands
        for key,item in supportedCmd.items():
            #print(key,item)
            def makeCmd(cmd):
                def makeSetCmd(self):
                    command = self.encondeCommand(cmd)
                    self.SetSerial(command)
                def makeGetCmd(self):
                    command = self.encondeCommand(cmd)
                    msgout = self.decodeMessage(self.GetSerial(command))
                    return msgout
                return makeSetCmd, makeGetCmd
            
            Setcmd,Getcmd = makeCmd(item[0])
    
            if 'W' in item[1]:
                setattr(self, key, Setcmd.__get__(self,self.__class__))
            if 'R' in item[1]:
                setattr(self, key, Getcmd.__get__(self,self.__class__))
        