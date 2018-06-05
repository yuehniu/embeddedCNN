/*
    Desc:
    
        Function set for CNN in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        06/04/2018

    Author:

        Yue Niu
*/

#include "../../include/fpga/cnn_fpga.h"

/*
  Top function for convolution in FPGA.

  Argument:
    
    In, pointer to input data.
    Out, pointer to output data.

  Note:
  
    Currently, only data copy in included to validate the data flow design.
*/
void conv_fpga(Dtype * In, Dtype * Out)
{
  Dtype in_buf[TILE_SIZE][TILE_SIZE];

  Dtype out_buf[TILE_SIZE][TILE_SIZE];

  Dtype * in_ptr = In;
  Dtype * out_ptr = Out;
  for(int tcnt = 0; tcnt < TILE_NUM; tcnt++){
  #pragma HLS DATAFLOW

    // Read input to in_buf
    buf_read(in_ptr + tcnt * TILE_SIZE * TILE_SIZE, in_buf);

    // Move in_buf to out_buf
    buf_move(in_buf, out_buf);

    // Write out_buf to Out
    buf_write(out_buf, out_ptr + tcnt * TILE_SIZE * TILE_SIZE);
  }

  return;
}

void buf_read(Dtype * In, Dtype In_Buf[TILE_SIZE][TILE_SIZE])
{
  for (int r = 0; r < TILE_SIZE; r++){
    for (int c = 0; c < TILE_SIZE; c++){
    #pragma HLS PIPELINE
      In_Buf[r][c] = *In++;
    }
  }

  return;
}

void buf_move(Dtype In_Buf[TILE_SIZE][TILE_SIZE], Dtype Out_Buf[TILE_SIZE][TILE_SIZE])
{
  for (int r = 0; r < TILE_SIZE; r++){
    for (int c = 0; c < TILE_SIZE; c++){
    #pragma HLS PIPELINE
      Out_Buf[r][c] = In_Buf[r][c];
    }
  }

  return;
}

void buf_write(Dtype Out_Buf[TILE_SIZE][TILE_SIZE], Dtype * Out)
{
  for (int r = 0; r < TILE_SIZE; r++){
    for (int c = 0; c < TILE_SIZE; c++){
    #pragma HLS PIPELINE
      *Out++ = Out_Buf[r][c];
    }
  }
}
