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
               int WISec,    // Input sec number in buf for each layer
               int PoolDiv,
               bool Pool     // Whether pooling
              )
{
  Dtype out_buf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC];
  #pragma HLS array_partition variable=out_buf complete dim=1

  Dtype pool_buf[OTILE][FTILE_W * O_BUF_SEC / 2];
  #pragma HLS array_partition variable=pool_buf complete dim=1

  Dtype b_buf[B_BUF_DEPTH];
  #pragma HLS array_partition variable=b_buf complete
 
  int row;
  int r = 0;
  int rowmod = 0;
  int out_offset = 0;

  /* Start Convolution */

  // Read bias first
  bias_read(Param, b_buf, OChnl);
  #ifdef CHECK_CPU
  // b_buf check
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
               ": Check BBuf." << std::endl;
  bbuf_check(Param, b_buf, OChnl);
  #endif
  Param += OChnl;
  for (row = 0; row < RowNum; row += FTILE_H){ /* Row loop */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=224 max=224

    conv_ichnl(In, Param, b_buf, out_buf,
               ChnlRead,
               OTILE,
               IChnl,
               OChnl,
               Kern,
               WISec,
               ISec,
               OSec, 
               row,
               Lyr,
               r,
               ColNum 
               );
  
  // Write on-chip data to external mem
  buf_write(out_buf, pool_buf, Out + out_offset, 
            rowmod,       // Which row should be write off
            row >= 1,     // Whether write
            Pool,         // Whether pooling
            !(row & 0x1), // Whether write pooling off
            ColNum,       // Col number in cur layer
            OSec
            );
  if(row >= 1){
    if (Pool){
      if(!(row & 0x1))  
        out_offset += OChnl * ColNum / 2;
    }
    else{
      out_offset += OChnl * ColNum;
    }
  }

  if (Kern == r) r = 1;
  else           r +=1;

  if ((Kern-1) == rowmod) rowmod = 0;
  else                    rowmod += 1;
 
  // Conv op 
  }/* Row loop */

  // Write the last row
  buf_write(out_buf, pool_buf, Out + out_offset, 
            rowmod,       // Which row should be write off
            true,         // Whether write
            Pool,         // Whether Pooling
            !(row & 0x1), // Whether write poolling off 
            ColNum,       // Col number in cur layer
            OSec
           );

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
              Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H],
              int ChnlNum,
              int ColNum)
{
  for (int n = 0; n < ChnlNum; n++){
  #pragma HLS loop_tripcount min=16 max=16
    for (int r = 0; r < FTILE_H; r++){
    #pragma HLS loop_tripcount min=1 max=1
      for (int c = 0; c < ColNum; c++){
      #pragma HLS loop_tripcount min=224 max=224
      #pragma HLS PIPELINE
        if ((0 == c) || (ColNum-1 == c))
          InBuf[n][r * ColNum + c] = (Dtype)0.0;
        else
          InBuf[n][r * ColNum + c] = *In++;
      }
    }
  }/* for: input channel tile */

  return;
}

void conv_ichnl(Dtype *In,
                Dtype *Param,
                Dtype BBuf[B_BUF_DEPTH],
                Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC],
                int IChnlTil,
                int OChnlTil,
                int IChnl,
                int OChnl,
                int Kern,
                int WISec,
                int ISec,
                int OSec,
                int Row,
                int Lyr,
                int RMod,
                int ColNum)
{
  // Set on-chip buffer
  Dtype in_buf[ITILE][(FTILE_W+2) * FTILE_H];
  #pragma HLS array_partition variable=in_buf complete dim=1

  for (int n = 0; n < ISec; n++){ /* Input channel */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=4 max=4
    // Read input feature
    buf_read(In + (Row * IChnl + (n<<ITILSHFT)) * ColNum, // incr In
             in_buf, 
             IChnlTil,
             ColNum+2); 
    #ifdef CHECK_CPU
    std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
                 ": Check InBuf." << std::endl;
    inbuf_check(In, in_buf, Lyr, row * ISec + n);
    #endif

    conv_ochnl(Param, in_buf, BBuf, OutBuf,
               IChnlTil,
               OTILE,
               OChnl,
               Kern,
               n,
               WISec,
               OSec,
               Row,
               Lyr,
               RMod,
               ColNum);
  }

  return;
}

void conv_ochnl(Dtype *Param,
                Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H],
                Dtype BBuf[B_BUF_DEPTH],
                Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC],
                int IChnlTil,
                int OChnlTil,
                int OChnl,
                int Kern,
                int Ni,
                int WISec,
                int OSec,
                int Row,
                int Lyr,
                int RMod,
                int ColNum)
{
  static Dtype w_buf[OTILE * ITILE][W_BUF_DEPTH];
  #pragma HLS array_partition variable=w_buf complete dim=1

  for(int m = 0; m < OSec; m++){ /* Output channel */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=4 max=4
    // Conditional read weights 
    if (0 == Row || (Lyr > 6))
      weight_read(Param + ((Ni<<ITILSHFT) * OChnl + (m<<OTILSHFT) * IChnlTil) * Kern * Kern, 
                  w_buf, 
                  IChnlTil,
                  OTILE,
                  Kern,
                  (Ni - ((Ni >> WISec) << WISec)) * OSec + m, // Read to which sec
                  true   // Whether to read
                  );
    #ifdef CHECK_CPU
    // w_buf check
    if ((0 == row)){
      std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": Check WBuf." << std::endl;
      wbuf_check(Param + ((Ni<<ITILSHFT) * OChnl + (m<<OTILSHFT) * IChnlTil) * Kern * Kern,
                 w_buf,
                 IChnlTil,
                 OTILE,
                 Kern,
                 (Ni - ((Ni >> WISec) << WISec)) * OSec + m);
    }
    #endif

    // Compute in parallel
    compute(InBuf, 
            w_buf, 
            BBuf, 
            OutBuf, 
            RMod,
            ColNum, 
            Kern, 
            IChnlTil, 
            OTILE, 
            OSec,
            (Ni - ((Ni >> WISec) << WISec)),
            m, 
            Ni == 0);

    //m_i += 1;
  }/* Output channel */  
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
  for (int m = 0; m < OTILE; m++) {
  #pragma HLS loop_tripcount min=32 max=32
   for (int n = 0; n < IChnlTil; n++) {
   #pragma HLS loop_tripcount min=16 max=16
     for (int k = 0; k < Kern * Kern; k++)
     #pragma HLS loop_tripcount min=9 max=9
     #pragma HLS PIPELINE
       if(Read)
         Wbuf[m * ITILE + n][Sec * Kern * Kern + k] = *Param++;
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
  Parallel computing unit

  Argument:

    InBuf, input data buffer
    WBuf,  weights buffer
    BBuf,  bias buffer
    OutBuf, output data buffer
    Row, current row
    ColNum, total col number in current layer
    Kern, kernel size
    IChnlTil, input channel tile number
    OChnlTil, output channel tile number
    OTilNum, total output channel tiles
    ISec, input channel sector
    OSec, output channel sector
    LoadBias, whether to load bias

  Note:

    - Current design is for k * k kernel size,
    not for k1 * k2 kernel.
    - Since input tile is just a row data, k row
    weights will applied to this signle row to generate
    three row partial output.

*/
void compute(Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H],
             Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
             Dtype BBuf[B_BUF_DEPTH],
             Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC ],
             int Row,
             int ColNum,
             int Kern,
             int IChnlTil,
             int OChnlTil,
             int OTilNum,
             int ISec,
             int OSec,
             bool LoadBias)
{
  // Set a partial sum reg array
  //Dtype pesum[OTILE][O_BUF_ROW];
  //#pragma HLS array_partition variable=pesum complete dim=1

  Dtype partial[OTILE];
  #pragma HLS array_partition variable=partial complete
  
  for (int col = 1; col < ColNum + 1; col++) { /* Col */
  #pragma HLS loop_tripcount min=224 max=224
    // Computing partial sum
    for (int k1 = 0; k1 < Kern; k1++) { /* Kernel row */
    #pragma HLS loop_tripcount min=3 max=3
      for (int k2 = 0; k2 < Kern; k2++) { /* Kernel col */
      #pragma HLS loop_tripcount min=3 max=3
      #pragma HLS PIPELINE
         for (int m = 0; m < OTILE; m++) { /* Out channel */
         //#pragma HLS UNROLL
         //#pragma HLS dependence variable=OutBuf inter false
           if(LoadBias && 0 == k2 && (0 == Row || ((Kern-Row) == k1)))  
             partial[m] = BBuf[OSec * OTILE + m];
           else
             partial[m] = 0.0;
           for (int n = 0; n < ITILE; n++) { /* In channel */
           //#pragma HLS UNROLL
             Dtype input = InBuf[n][col - 1 + k2];

             Dtype weight = 0.0;
             if ((k1+Row) < Kern)
               weight = WBuf[m * ITILE + n]
                            [ISec * OTilNum * Kern * Kern + 
                             OSec * Kern * Kern + 
                             (k1 + Row) * Kern + 
                              k2
                            ];
             else
               weight = WBuf[m * ITILE + n]
                            [ISec * OTilNum * Kern * Kern + 
                             OSec * Kern * Kern + 
                             ((k1 + Row) - Kern) * Kern + 
                             k2
                            ];
             
             partial[m] += weight * input;

             //if(3 == col - 1 && 118 == (OSec*OTILE+m) )
             //  std::cout << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
             //               ": " << OSec*OTILE + m << "th ochnl," <<
             //                       Row+k1 << "th row, " <<
             //                       col-1 << "th col, " <<
             //                       ISec << "th ichnl, " <<
             //                       k2 << "th weight, " <<
             //                       input << "--" << weight <<
             //                       " mul value, " << partial[m] <<
             //                       std::endl;
           } /* In channel */

           if(LoadBias && 0 == k2 && 
              (0 == Row || 
               (1 == Row && (Kern-1) == k1) ||
               (k1 == (Kern - Row))
              )
             )
           {
             OutBuf[m][OSec * O_BUF_ROW * ColNum + k1 * ColNum + col - 1] = 
               partial[m];
           }
           else {
             OutBuf[m][OSec * O_BUF_ROW * ColNum + k1 * ColNum + col - 1] += 
               partial[m];
           }
       
           //if(3 == col - 1 && 118 == (OSec*OTILE+m) )
           //  std::cout << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
           //               ": accum data, " << 
           //               OutBuf[m][OSec * O_BUF_ROW * ColNum + k1 * ColNum + col - 1] <<
           //               std::endl;
         } /* Out channel */
      } /* Kernl col */ 
    } /* Kernel row */ 
                  
  } /* Col */
  return;
}
/*
  Write buf data to extern mem

  Arguments:

    OutBuf, output buffer
    Out, external output mem
    Row, row % Kern
    Write, whether to write data out
    ColNum, col number in one layer
    OSec,  output channel sector

  Note:
  
    This function contrain relu and max pooling op. 
*/
void buf_write(Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC], 
               Dtype PoolBuf[OTILE][FTILE_W * O_BUF_SEC / 2],
               Dtype *Out, 
               int Row,
               bool Write,
               bool Pool,
               bool PoolWrite,
               int ColNum,
               int OSec)
{
  for (int m = 0; m < OSec; m++){
    for (int m_i = 0; m_i < OTILE; m_i++){
      for (int col = 0; col < ColNum; col+=2) {
      #pragma HLS PIPELINE
        if (Write) {
          Dtype data1 = 0.0;
          Dtype data2 = 0.0;
          if (0 == Row) {
            data1 = OutBuf[m_i][m * O_BUF_ROW * ColNum + 2 * ColNum + col];
            data2 = OutBuf[m_i][m * O_BUF_ROW * ColNum + 2 * ColNum + col + 1];
          }
          else if (1 == Row) {
            data1 = OutBuf[m_i][m * O_BUF_ROW * ColNum + ColNum + col];
            data2 = OutBuf[m_i][m * O_BUF_ROW * ColNum + ColNum + col + 1];
          }
          else if (2 == Row) {
            data1 = OutBuf[m_i][m * O_BUF_ROW * ColNum + col];
            data2 = OutBuf[m_i][m * O_BUF_ROW * ColNum + col + 1];
          }
          if (Pool){
            if (PoolWrite){
              Dtype max_cur = (data1 > data2) ? data1 : data2;
              Dtype max_last = PoolBuf[m_i][m * (ColNum >> 1) + (col >> 1)];
              Dtype max = (max_cur > max_last) ? max_cur : max_last;
              *Out++ = (max < 0) ? (Dtype)0.0 : max;
            }
            else{
              Dtype max = (data1 > data2) ? data1 : data2;
              PoolBuf[m_i][m * (ColNum >> 1) + (col >> 1)] = max;
            }
          }
          else {
            *Out++ = (data1 < 0) ? (Dtype)0.0 : data1;
            *Out++ = (data2 < 0) ? (Dtype)0.0 : data2;
          }
        }
      } 
    }
  }

  return;
}
