/*
    Desc:
    
        Conv Op in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        06/04/2018

    Author:

        Yue Niu
*/

#include "../../include/fpga/conv_fpga.h"

/*
  Top function for convolution in FPGA.
  In order to save data transfer bandwidth, ReLU and optional Pooling
  will be executed after convolution. 

  Argument:
    
    In, pointer to input data.
    Out, pointer to output data.

  Note:
  
    Currently, only data copy in included to validate the data flow design.
*/
void conv_fpga(Dtype * In, Dtype * Out)
{
  Dtype in_buf[FTILE][FTILE];

  Dtype out_buf[FTILE][FTILE];

  Dtype * in_ptr = In;
  Dtype * out_ptr = Out;
  for(int tcnt = 0; tcnt < TILE_NUM; tcnt++){
  #pragma HLS DATAFLOW

    // Read input to in_buf
    buf_read(in_ptr + tcnt * FTILE * FTILE, in_buf);

    // Move in_buf to out_buf
    buf_move(in_buf, out_buf);

    // Write out_buf to Out
    buf_write(out_buf, out_ptr + tcnt * FTILE * FTILE);
  }

  return;
}

/*
  Top function for convolution in FPGA.
  In order to save data transfer bandwidth, ReLU and optional Pooling
  will be executed after convolution. 

  Argument:
    
    In, pointer to input data.
    Params, pointer to weights and bias
    Out, pointer to output data.

  Note:
  
    Currenly, real conv should be implemented.
*/
void conv_fpga(Dtype *In, Dtype *Params, Dtype *Out)
{
  Dtype in_buf[ITILE][FTILE * FTILE];
  Dtype out_buf[OTILE][FTILE * FTILE];
  Dtype w_buf[OTILE * ITILE][]
  Dtype b_buf[OTILE];
  return;
}

void buf_read(Dtype * In, Dtype In_Buf[FTILE][FTILE])
{
  for (int r = 0; r < FTILE; r++){
    for (int c = 0; c < FTILE; c++){
    #pragma HLS PIPELINE
      In_Buf[r][c] = *In++;
    }
  }

  return;
}

void buf_move(Dtype In_Buf[FTILE][FTILE], Dtype Out_Buf[FTILE][FTILE])
{
  for (int r = 0; r < FTILE; r++){
    for (int c = 0; c < FTILE; c++){
    #pragma HLS PIPELINE
      Out_Buf[r][c] = In_Buf[r][c];
    }
  }

  return;
}

void buf_write(Dtype Out_Buf[FTILE][FTILE], Dtype * Out)
{
  for (int r = 0; r < FTILE; r++){
    for (int c = 0; c < FTILE; c++){
    #pragma HLS PIPELINE
      *Out++ = Out_Buf[r][c];
    }
  }
}
