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

#include "sds_lib.h"

#include "../../include/fpga/conv_fpga.h"
#include "../../include/common.h"

#include "../../include/utils/check.h"

/*
  Top function for convolution in FPGA.
  In order to save data transfer bandwidth, ReLU and optional Pooling
  will be executed after convolution. 

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
              0      1    ...   15      16     17         31    ...   512
    ^  ^   |0-0-0 |0-1-0 |...|0-15-0 |0-0-1 |0-1-1 |...|0-15-1 |...|0-15-15|
    |  |   |1-0-0 |1-1-0 |...|1-15-0 |1-0-1 |1-1-1 |...|1-15-1 |...|1-15-15|
    | sec0 |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |  v   |9-0-0 |9-1-0 |...|9-15-0 |9-0-1 |9-1-1 |...|9-15-1 |...|9-15-15|
   288     |0-0-16|0-1-16|...|0-15-16|0-0-17|0-1-17|...|0-15-17|...|0-15-31|
    |      |1-0-16|1-1-16|...|0-15-16|1-0-17|1-1-17|...|1-15-17|...|1-15-31|
    |      |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    |      |9-0-16|9-1-16|...|0-15-16|9-0-17|9-1-17|...|9-15-17|...|9-15-31|
    v      |...   |...   |...|...    |...   |...   |...|...    |...|...    |
    
*/

void conv_fpga(Dtype *In,    // Variable DMA transfer length 
               Dtype *Param, // Variable DMA transfer length
               Dtype *Out,   // Varaieble DMA transfer length
               int Lyr,      // Current layer index 
               int RowNum,   // Row number
               int ColNum,   // Col number
               int ChnlRead, // Input channel to read in each tile
               int Kern,     // Kernel size
               int IChnl,    // Input param channel to read
               int ISec,     // Input sec number
               int OChnl,    // Output param channel to read
               int OSec,     // Out sec number
               int WISec     // Input sec number in buf for each layer
              )
{
  // Set on-chip buffer
  Dtype in_buf[ITILE][FTILE_W * FTILE_H];
  #pragma HLS array_partition variable=in_buf complete dim=1

  // Dtype out_buf[O_BUF_DEPTH][O_BUF_ROW * FTILE_W * FTILE_H];
  // int fac = OTILE;
  // #pragma HLS array_partition variable=out_buf cyclic factor=fac dim=1

  Dtype w_buf[OTILE * ITILE][W_BUF_DEPTH];
  #pragma HLS array_partition variable=w_buf complete dim=1

  Dtype b_buf[B_BUF_DEPTH];
  #pragma HLS array_partition variable=b_buf complete

  /* Start Convolution */
  // int chnl_num = Lyr == 0 ? 3 : CHNEL[Lyr - 1]; 
  // Read bias first
  bias_read(Param, b_buf, OChnl);
  #ifdef CHECK_CPU
  // b_buf check
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
               ": Check BBuf." << std::endl;
  bbuf_check(Param, b_buf, OChnl);
  #endif
  Param += OChnl;
  for (int row = 0, col = 0; row < RowNum; row += FTILE_H){
  // #pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=224 max=224
    for (int n = 0; n < IChnl; n += ITILE) { 
    // #pragma HLS DATAFLOW
    #pragma HLS loop_tripcount min=1 max=1
    /* Input channel */
      // Read input feature
      buf_read(In + (row * IChnl + n) * FTILE_W, // incr In
               in_buf, 
               ChnlRead,
               ColNum); 
      #ifdef CHECK_CPU
      std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
                   ": Check InBuf." << std::endl;
      inbuf_check(In, in_buf, Lyr, row);
      #endif
      for (int m = 0, m_i = 0; m < OChnl; m += OTILE, m_i++){ 
      // #pragma HLS DATAFLOW
      #pragma HLS loop_tripcount min=2 max=2
      /* Output channel */
        // Conditional read weights 
        weight_read(Param + (n * OChnl + m * ChnlRead) * Kern * Kern, 
                    w_buf, 
                    ChnlRead,
                    OTILE,
                    Kern,
                    ((n/ITILE) % WISec) * OSec + m_i, // Read to which sec
                    (0 == row && 0 == col) || (Lyr > 3)   // Whether to read
                    );
        #ifdef CHECK_CPU
        // w_buf check
        if ((0 == row && 0 == col) || (Lyr > 3)){
          std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                       ": Check WBuf." << std::endl;
          wbuf_check(Param + (n + m * ChnlRead) * Kern * Kern,
                     w_buf,
                     ChnlRead,
                     OTILE,
                     Kern,
                     ((n/ITILE) % WISec) * OSec + m_i);
        }
        #endif
      }/* Output channel */  
    } /* Input channel */

  // Conv op 
  }/* Start Convolution */

  // Write on-chip data to external mem
  buf_write(b_buf, Out, OChnl);
  return;
}

/*
  Read inputs from external to in_buf.

  Argument:

    In, pointer to externel mem.
    InBuf, on-chip buffer.
    ChnlNum, input channels to read.
    ColNum, col number to read.

  Note:

    In current design, stride was set to be default 1.
    If stride is not 1, the code should be modified.
*/
void buf_read(Dtype * In, 
              Dtype InBuf[ITILE][FTILE_W * FTILE_H],
              int ChnlNum,
              int ColNum)
{
  for (int n = 0; n < ChnlNum; n++){
  #pragma HLS loop_tripcount min=3 max=3
    for (int r = 0; r < FTILE_H; r++){
    #pragma HLS loop_tripcount min=1 max=1
      for (int c = 0; c < ColNum; c++){
      #pragma HLS loop_tripcount min=224 max=224
      #pragma HLS PIPELINE
        InBuf[n][r * ColNum + c] = *In++;
      }
    }
  }/* for: input channel tile */

  return;
}

/*
  Read weights from externel to w_buf
  
  Note:
  
    In this design, only a sector (like 3 * 32 * 9)
    data was read into w_buf each cycles.
    Moreover, in some layer, w_buf can accomodate all
    the weights, so we needn't to reread weights from
    external again in next few cycles.
  
*/
void weight_read(Dtype *Param, 
                 Dtype Wbuf[OTILE * ITILE][W_BUF_DEPTH],
                 int IChnlTil, // Input channel tile to read
                 int OChnlTil, // Output channel tile to read
                 int Kern,     // Kernel size
                 int Sec,      // Start sec position
                 bool Read     // Wheter to read
                )
{
  for (int m = 0; m < OChnlTil; m++) {
  #pragma HLS loop_tripcount min=32 max=32
   for (int n = 0; n < IChnlTil; n++) {
   #pragma HLS loop_tripcount min=3 max=3
     for (int k = 0; k < Kern * Kern; k++)
     #pragma HLS loop_tripcount min=9 max=9
     #pragma HLS PIPELINE
       if(Read)
         Wbuf[m * IChnlTil + n][Sec * Kern * Kern + k] = *Param++;
   }
  }

  return;
}

/*
  Read bias from extern to b_buf

  Argument:
  
    Param, pointer to externel param
    Bbuf, on-chip bias buffer
    OChnl,output channel to read 
*/
void bias_read(Dtype *Param, Dtype Bbuf[B_BUF_DEPTH], int OChnl)
{
  for (int n = 0; n < OChnl; n++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=64 max=64
    Bbuf[n] = *Param++;
  }

  return;
}

/*
  Write buf data to extern mem

  Note:
 
    This function was used to verify on-chip dataflow,
    by write on-chip data to external mem, and check
    data in CPU.
*/
void buf_write(Dtype OutBuf[B_BUF_DEPTH], Dtype *Out, int OChnl)
{
  for (int n = 0; n < OChnl; n++){
  #pragma HLS PIPELINE
    *Out++ = OutBuf[n];
  }

  return;
}
