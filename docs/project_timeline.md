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
