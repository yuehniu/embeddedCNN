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
    TilNum, total tile number in currenly layer

  Note:
  
    in_buf mem arrangement(chnl-pos):
    0  |0-0  |1-0  |2-0  |...|15-0  |
    1  |0-1  |1-1  |2-1  |...|15-0  |
    2  |0-2  |1-2  |2-2  |...|15-0  |
       |...  |...  |...  |...|...   |
    441|0-441|1-441|2-441|...|15-441|

    out_buf mem arrangement(chnl-pos):
       0       1       2     ...    15
    |0-0    |1-0    |2-0    |...|15-0   |
    |0-1    |1-1    |2-1    |...|15-0   |
    |0-2    |1-2    |2-2    |...|15-0   |
    |...    |...    |...    |...|...    |
    |0-441  |0-441  |2-441  |...|15-441 |
    |16-0   |17-0   |18-0   |...|31-0   |
    |16-1   |17-1   |18-1   |...|31-0   |
    |16-2   |17-2   |18-2   |...|31-0   |
    |...    |...    |...    |...|...    |
    |16-441 |17-441 |18-441 |...|31-441 |
    |...    |...    |...    |...|...    |
    |496-0  |497-0  |498-0  |...|511-0  |
    |496-1  |497-1  |498-1  |...|511-0  |
    |496-2  |497-2  |498-2  |...|511-0  |
    |...    |...    |...    |...|...    |
    |496-441|497-441|498-441|...|511-441|

    w_buf mem arrangement(pos-ichnl-ochnl)
       0      1    ...   15      16     17         31    ...   255
    |0-0-0 |0-1-0 |...|0-15-0 |0-0-1 |0-1-1 |...|0-15-1 |...|0-15-15|
    |1-0-0 |1-1-0 |...|1-15-0 |1-0-1 |1-1-1 |...|1-15-1 |...|1-15-15|
    |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |9-0-0 |9-1-0 |...|9-15-0 |9-0-1 |9-1-1 |...|9-15-1 |...|9-15-15|
    |0-0-16|0-1-16|...|0-15-16|0-0-17|0-1-17|...|0-15-17|...|0-15-31|
    |1-0-16|1-1-16|...|0-15-16|1-0-17|1-1-17|...|1-15-17|...|1-15-31|
    |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |9-0-16|9-1-16|...|0-15-16|9-0-17|9-1-17|...|9-15-17|...|9-15-31|
    |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    
*/
void conv_fpga(Dtype *In, Dtype *Params, Dtype *Out, int TilNum)
{
  // Set on-chip buffer
  Dtype in_buf[ITILE][FTILE * FTILE];
  #pragma HLS array_partition variable=in_buf block complte dim=1

  Dtype out_buf[O_BUF_DEPTH][FTILE * FTILE];
  #pragma HLS array_partition variable=out_buf cyclic factor=OTILE dim=1

  Dtype w_buf[OTILE * ITILE * W_BUF_DEPTH];
  #pragma HLS array_reshape variable=w_buf cyclic factor=ITILE*OTILE dim = 1

  Dtype b_buf[OTILE];
  #pragma HLS array_partition variable=b_buf complete

  // Compute total tile number
  total_tile = 

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
