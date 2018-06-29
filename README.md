# embeddedCNN
Deploy CNN accelerator in embedded OS using SDSOC and Xilinx Ultrascale+ platform.

## Platform

SDx: 2018.1 

Board: Xilinx Ultrascale+ ZCU 102

## FPGA system

Data type: only float16 now!

## Version

conv.v0.0: 
  
 This version takes long run time (about 11s). Since, parameters in certain conv layer cannot be fully loaded into on-chip mem, they have to be readed many times from DDR to FPGA. Therefore, it takes much time on the data communication.
