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
/*
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
*/

/*
  Top function for convolution in FPGA.
  In order to save data transfer bandwidth, ReLU and optional Pooling
  will be executed after convolution. 

  Argument:
    
    In, pointer to input data.
    Params, pointer to weights and bias
    Out, pointer to output data.
    Lyr, current convolution layer
    TilNum, total tile number in current layer
    ChnlTilNum: total channel tile number in current layer

  Note:
  
    in_buf mem arrangement(chnl-pos):
    0  |0-0  |1-0  |2-0  |...|15-0  |
    1  |0-1  |1-1  |2-1  |...|15-0  |
    2  |0-2  |1-2  |2-2  |...|15-0  |
       |...  |...  |...  |...|...   |
    224|0-224|1-224|2-224|...|15-224|

    out_buf mem arrangement(chnl-pos):
           0       1       2     ...    15
    ^   |0-0    |1-0    |2-0    |...|15-0   |
    |   |0-1    |1-1    |2-1    |...|15-0   |
    |   |0-2    |1-2    |2-2    |...|15-0   |
    |   |...    |...    |...    |...|...    |
    |   |0-441  |0-441  |2-441  |...|15-441 |
    |   |16-0   |17-0   |18-0   |...|31-0   |
    |   |16-1   |17-1   |18-1   |...|31-0   |
  32*441|16-2   |17-2   |18-2   |...|31-0   |
    |   |...    |...    |...    |...|...    |
    |   |16-441 |17-441 |18-441 |...|31-441 |
    |   |...    |...    |...    |...|...    |
    |   |496-0  |497-0  |498-0  |...|511-0  |
    |   |496-1  |497-1  |498-1  |...|511-0  |
    |   |496-2  |497-2  |498-2  |...|511-0  |
    |   |...    |...    |...    |...|...    |
    v   |496-441|497-441|498-441|...|511-441|

    w_buf mem arrangement(pos-ichnl-ochnl)
          0      1    ...   15      16     17         31    ...   255
    ^  |0-0-0 |0-1-0 |...|0-15-0 |0-0-1 |0-1-1 |...|0-15-1 |...|0-15-15|
    |  |1-0-0 |1-1-0 |...|1-15-0 |1-0-1 |1-1-1 |...|1-15-1 |...|1-15-15|
    |  |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |  |9-0-0 |9-1-0 |...|9-15-0 |9-0-1 |9-1-1 |...|9-15-1 |...|9-15-15|
   288 |0-0-16|0-1-16|...|0-15-16|0-0-17|0-1-17|...|0-15-17|...|0-15-31|
    |  |1-0-16|1-1-16|...|0-15-16|1-0-17|1-1-17|...|1-15-17|...|1-15-31|
    |  |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |  |9-0-16|9-1-16|...|0-15-16|9-0-17|9-1-17|...|9-15-17|...|9-15-31|
    v  |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    
*/
extern const int CHNEL[];
extern const int SHAPE[];
void conv_fpga(Dtype *In, Dtype *Params, Dtype *Out, 
               int Lyr, int TilNum, int ChnlTilNum)
{
  // Set on-chip buffer
  Dtype in_buf[ITILE][FTILE_W * FTILE_H];
  #pragma HLS array_partition variable=in_buf block complte dim=1

  Dtype out_buf[O_BUF_DEPTH][FTILE_W * FTILE_H];
  #pragma HLS array_partition variable=out_buf cyclic factor=OTILE dim=1

  Dtype w_buf[OTILE * ITILE * W_BUF_DEPTH];
  #pragma HLS array_reshape variable=w_buf cyclic factor=ITILE*OTILE dim = 1

  Dtype b_buf[OTILE];
  #pragma HLS array_partition variable=b_buf complete

  /* Start Convolution */
  Dtype *in_ptr = In;
  Dtype *out_ptr = Out;
  // Set on-chip mem arrangement
  int chnl_num = Lyr == 0 ? 3 : CHNEL[Lyr - 1]; 
  int col_num = SHAPE[Lyr];
  // Rows to be read
  int row_num = 3;
  // Rows being valid
  int row_valid = 1;
  // Rows being valid in last row of input
  int last_row_valid = 1;
  int chnl_to_read = Lyr == 0 ? 3 : ITILE;
  switch (Lyr){
    case 0, 1 : { 
      row_num = 3;  
      row_valid = 1;  
      last_row_valid = 1;
      break;
    }
    case 2, 3 : {
      row_num = 6;  
      row_valid = 2;  
      last_row_valid = 2;
      break;
    }
    case 4, 5, 6 : {
      row_num = 12; 
      row_valid = 4;  
      last_row_valid = 4;
      break;
    }
    case 7, 8, 9 : {
      row_num = 24; 
      row_valid = 8;  
      last_row_valid = 4;
      break;
    }
    case 10, 11, 12 : {
      row_num = 14; 
      row_valid = 16; 
      last_row_valid = 14;
      break;
    }
    default: {
      row_num = 3;  
      row_valid = 1;  
      break;
    }
  }
  for (int til = 0; til < TilNum; til++){
  #pragma HLS DATAFLOW
    // Read input feature
    int fst = TilNum < ChnlTilNum; 
    int last_row = Til >= (TilNum - ChnlTilNum);
    buf_read(in_ptr, in_buf, chnl_to_read, row_num, col_num, row_valid, fst);

    // Conditional read weight

    // Conv op 
  }/* Start Convolution */

  return;
}

/*
  Read inputs from external to in_buf.

  Argument:

    In, pointer to in_buf.
    InBuf, on-chip buffer.
    ChnlNum, input channels to read.
    RowNum, row number to read.
    ColNum, col number to read.
    RowValid, row shift for bram during reading data,
             also, is the number of valid rows.
    Fst, whether to read first line. 

  Note:

    In current design, stride was set to be default 1.
    If stride is not 1, the code should be modified.
*/
void buf_read(Dtype * In, Dtype InBuf[ITILE][FTILE_W * FTILE_H],
              int ChnlNum, int RowNum, int ColNum, 
              int RowValid, bool Fst)
{
  for (int n = 0; n < ChnlNum; n++){
    for (int r = 0; r < RowNum; r++){
      for (int c = 0; c < ColNum; c++){
      #pragma HLS PIPELINE
      #pragma HLS dependence variable=InBuf inter false
        // First line?
        if (true == Fst){
          if(r > 0)
            InBuf[n][r * c] = *In++;
          else
            InBuf[n][r * c] = 0.0;
        }
        else {
          if (r >= RowValid) 
            InBuf[n][r * c - FTILE_W] = InBuf[n][r * c]
          if (r >= RowNum - RowValid)
            InBuf[n][r * c] = *In++;
        }/* First line? */
      }
    }
  }/* for: input channel tile */

  return;
}

void buf_move(Dtype In_Buf[FTILE_W][FTILE_H], Dtype Out_Buf[FTILE_W][FTILE_H])
{
  for (int r = 0; r < FTILE_H; r++){
    for (int c = 0; c < FTILE_W; c++){
    #pragma HLS PIPELINE
      Out_Buf[r][c] = In_Buf[r][c];
    }
  }

  return;
}

void buf_write(Dtype Out_Buf[FTILE_W][FTILE_H], Dtype *Out)
{
  for (int r = 0; r < FTILE_H; r++){
    for (int c = 0; c < FTILE_W; c++){
    #pragma HLS PIPELINE
      *Out++ = Out_Buf[r][c];
    }
  }
}
