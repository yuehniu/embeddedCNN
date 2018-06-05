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