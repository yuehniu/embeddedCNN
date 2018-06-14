# Embedded CNN in Xilinx FPGA

## Regular timeline

### 06/05/2018

- Simple dataflow framework and check code finished.

  The dataflow framework consists three parts: 

    * Read data from ARM CPU memory to FPGA buf_in
    * Move data from buf_in to FPGA buf_out
    * Write data from FPGA buf_out to ARM CPU memory

- Image data binary file generated.

  Details:
  
    * By using Python, image binary data were extracted from
    Caffe.

### 06/07/2018

- Half float point operation implemented.

  When I try to modify the example design "Matrix Multiplication and Add",
  using half float to replace single float. Af the begining, it failed with
  error **Error: DMASIMPLE could not find physical address for buffer - make
  sure the buffer is allocated with sds_alloc**. Then I try to modify the SDS
  pragma in header file, as shown follows. It works.

  Original: 

  ```c
  void mmult (Dtype A[N*N], Dtype B[N*N], Dtype C[N*N]);
  ```

  To: 

  ```c
  #pragma SDS data mem_attribute(A:PHYSICAL_CONTIGUOUS, B:PHYSICAL_CONTIGUOUS, C:PHYSICAL_CONTIGUOUS)
  void mmult (Dtype *A, Dtype *B, Dtype *C);
  ```

  Interesting observations:

    For 64x64 matrix mul and add using fp16, ARM CPU takes 14367058 cpu 
    cycles, while Xilinx FPGA takes 165974 cpu cycles, which is 86 times more faster than ARM CPU. 

    On the other side, For 64x64 matrix mul and add using fp32, ARM CPU takes 
    627638 cpu cycles, while Xilinx FPGA takes 168804 cpu cycles, which is 3.7 times more faster than ARM CPU.

### 06/12/2018

- Big problem encountered!

  Currently, buffer for input feature was set to be 16 * 3 * 224. In the next 
  execution cycle, a new next line will be read into the buffer, meanwhile, 2 
  bottom line will be push upward. However, the problem is: the design will 
  not iterate over **row**, it iterate over **input channel**. Therefore, in 
  next execution cycle, new data from other **input channels** will be read 
  into the buffer. The whole data will replaced by new data. When it come back
  to the first **input channel tile**, all the previous data is gone. We have 
  to again read the needed data from DDR. For 3x3 kernel, we have to read two 
  tow already used data from DDR.

  In order to solve this problem, I plan to design a 16 * 1 * 224 input buffer 
  For one execution pipeline cycle, I do all the conv ops on the 16 * 1 * 224 
  data. In normal cases, I need to calculate 2th row conv for ith row output, 
  1th row conv for (i+1)th row output, 0th row conv for (i+2)th row output. 
  I'll write down detailed implementation later.

### 06/14/2018

- TIle-based loop is a bad design!
  
  Previously, I tired to used tile-based loop to realize a **DATAFLOW** style 
  processing, in which all the iteration can be combined to one loop. This 
  make **DATAFLOW** easy to realize. However, today, I realize that not all 
  the ops was executed in the one same tile loop. Like, **buf_read**, 
  **weight_read**, **write**, they are executed in different loop hierarchy. 
  Therefore, it's better to go back to original row-, col- channel-based 
  loop to conveniently arrange different ops into different loop hierarchy.