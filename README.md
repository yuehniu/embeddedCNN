# embeddedCNN
Deploy CNN accelerator in embedded OS using SDSOC and Xilinx Ultrascale+ platform.

## Platform

SDx: 2018.1 

Board: Xilinx Ultrascale+ ZCU 102

## FPGA system

Data type: only float16 now!  
Working frequency: 300MHz  
Data motion network frequency: 300MHz 

## Version

conv.v0.0: 
  
  This version takes long run time (about 11s). Since, parameters in certain conv layer cannot be fully loaded into on-chip mem, 
they have to be readed many times from DDR to FPGA. Therefore, it takes much time on the data communication.

conv.v0.1:

  This version take 9s to run conv layers. Compared to conv.v0.0, weight buffer is larger to read more weights into on-chip mem.
For, conv1/conv2/conv3, all the weights can be load into on-chip mem one time. For conv4/conv5, the weight buffer can only read 
1/4 weights one time. Therefore, in conv4/conv5, it take much more time than previos layers.

conv.v1.0:

  This version is much different with previous version. The main different is the input buffer arrangement. In previous version,
input buffer can only read one row of data, no matter how many cols it has. In this version, many rows of data will be read into
input buffer. For example, in conv1, 4 rows of data will be read one time; in conv2, it is 8 rows; in conv3, it is 16 rows; for
conv4/conv5 all the rows will be read one time. The design significantly improve the on-chip buffer efficiency. Furthermore, it
greatly reduces the runtime.

fc.v0.0:

  Fully-connected layer implemetation for VGGNet. It adopt pipeline arch to to matrix-vector multiplication. There are total 64
parallel pes.

conv.v0.0:

  This version targets for VGGNet-16. It takes 1.2s to finish total VGGNet-16, 
  running under 200MHz.

conv.v0.1:

  This version target for VGGNet-16. It takes 700ms to finish total VGGNet-16, 
  running under 300MHz.
