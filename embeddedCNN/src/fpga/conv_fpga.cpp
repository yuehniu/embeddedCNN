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
  
*/

void conv_fpga(Dtype *In,    // Variable DMA transfer length 
               Dtype *Param, // Variable DMA transfer length
               Dtype *Out,   // Varaieble DMA transfer length
               int Lyr,      // Current layer index 
               int RowNum,   // Row number
               int ColNum,   // Col number
               int Kern,     // Kernel size
               int IChnl,    // Input param channel to read
               int ISec,     // Input sec number
               int OChnl,    // Output param channel to read
               int OSec,     // Out sec number
               int TilNum,   // Total tile number
               int WISec,    // Input sec number in buf for each layer
               int PoolDiv,
               bool Pool     // Whether pooling
              )
{
  Dtype out_buf[OTILE][O_BUF_DEPTH];
  #pragma HLS array_partition variable=out_buf complete dim=1

  //Dtype pool_buf[OTILE][FTILE_W * O_BUF_SEC / 2];
  //#pragma HLS array_partition variable=pool_buf complete dim=1

  Dtype b_buf[B_BUF_DEPTH];
  #pragma HLS array_partition variable=b_buf complete
 
  int out_offset = 0;
  int itile = Lyr == 0 ? 3 : ITILE;
  int rows_read = 0;
  int rows_last = 0;

  /* Start Convolution */
  switch(Lyr){
    case 0: 
    case 1: {rows_read = 2; rows_last = 1; break;} 
    case 2:
    case 3: {rows_read = 6; rows_last = 3; break;}
    case 4:
    case 5:
    case 6: {rows_read = 14; rows_last = 13; break;}
    case 7:
    case 8:
    case 9: {rows_read = 28; rows_last = 28; break;}
    case 10:
    case 11:
    case 12: {rows_read = 14; rows_last = 14; break;}
    default: {rows_read = 2; break;}
  }

  // Read bias first
  bias_read(Param, b_buf, OChnl);
  #ifdef CHECK_CPU
  // b_buf check
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
               ": Check BBuf." << std::endl;
  bbuf_check(Param, b_buf, OChnl);
  #endif
  Param += OChnl;
  for (int til = 0; til < TilNum; til++) { /* Row loop */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=224 max=224

    int rows_valid =  Lyr > 6 ? rows_read : 
                      (0 == til ? rows_read + 1 : 
                      (TilNum-1 == til ? rows_last : 
                      rows_read));
                     
    int rows_new = 0 == til ? rows_read + 1 : rows_read;
    int rows_pre = 0 == til ? 1 : 2;
    int rows = rows_new + rows_pre;
    int rows_compute = TilNum-1 == til ? rows_last + rows_pre + 1: rows;
    int rows_write = (TilNum-1 == til ? rows_last + rows_pre + 1: rows) - Kern + 1;

    conv_ichnl(In, Param, b_buf, out_buf,
               itile,
               OTILE,
               IChnl,
               OChnl,
               Kern,
               WISec,
               ISec,
               OSec, 
               Lyr,
               til,
               ColNum,
               rows_new,
               rows_valid,
               rows_compute
               );

    In += IChnl * rows_valid * ColNum;
    
    // Write on-chip data to external mem
    buf_write(out_buf, Out + out_offset, 
              Pool,         // Whether pooling
              rows_write,
              ColNum,       // Col number in cur layer
              OSec
              );
    if (Pool){
      out_offset += OChnl * rows_write * ColNum >> 2;
    }
    else{
      out_offset += OChnl * rows_write * ColNum;
    }

    // Conv op 
  }/* Row loop */

  return;
}

/*
  Read inputs from external to in_buf.

  Argument:

    In, pointer to externel mem.
    InBuf, on-chip buffer.
    IChnlTil, input channel to read.
    IChnlTil, total input channels.
    Sec, input channel sector.
    RowsPre, rows read from pre-cycles.
    RowsRead, rows read in current cycles.
    RowsValid, rows valid.
    ColNum, col number to read.

  Note:

    - In current design, stride was set to be default 1.
    If stride is not 1, the code should be modified.
    - In different layer, different number of rows might
    be read to improve ram efficiency. 
*/
void buf_read(Dtype * In, 
              Dtype InBuf[ITILE][I_BUF_DEPTH],
              int IChnlTil,
              int IChnl,
              int Sec,
              int RowsPre,
              int RowsRead,
              int RowsValid,
              int ColNum)
{
  static Dtype in_pre[ITILE][I_PRE_DEPTH];

  for (int n = 0; n < IChnlTil; n++){
  #pragma HLS loop_tripcount min=16 max=16
    for (int r = 0; r < RowsPre; r++){
    #pragma HLS loop_tripcount min=1 max=1
      for (int c = 0; c < ColNum; c++){
      #pragma HLS loop_tripcount min=224 max=224
      #pragma HLS PIPELINE
        if (1 == RowsPre)
          InBuf[n][r * ColNum + c] = (Dtype)0.0;
        else
          InBuf[n][r * ColNum + c] = 
            in_pre[n][Sec * 2 * ColNum + r * ColNum + c];
      }
    }
  }/* for: input channel tile */

  for (int n = 0; n < IChnlTil; n++){
  #pragma HLS loop_tripcount min=16 max=16
    for (int r = 0; r < RowsRead; r++){
    #pragma HLS loop_tripcount min=1 max=1
      for (int c = 0; c < ColNum; c++){
      #pragma HLS loop_tripcount min=224 max=224
      #pragma HLS PIPELINE
        if (r < RowsValid){
          if ((0 == c) || (ColNum-1 == c)){
            InBuf[n][(r+RowsPre) * ColNum + c] = (Dtype)0.0;
            if (r >= RowsValid - 2)
              in_pre[n][Sec * 2 * ColNum + (r-RowsValid+2 ) * ColNum + c] =
                (Dtype)0.0;
          }
          else{
            Dtype data = *(In + r * IChnl * (ColNum-2) + n * (ColNum-2) + c-1);
            InBuf[n][(r+RowsPre) * ColNum + c] = data;
            if (r >= RowsValid - 2)
              in_pre[n][Sec * 2 * ColNum + (r-RowsValid+2 ) * ColNum + c]  = 
                data;
          }
        }
        else 
          InBuf[n][(r+RowsPre) * ColNum + c] = (Dtype)0.0;
      }
    }
  }/* for: input channel tile */

  return;
}

/*
  Conv entry point in input channel.

  Argument:

    In, input data.
    Param, parameters.
    BBuf, bias buffer.
    OutBuf, output buffer.
    IChnlTil, input channel tile.
    OChnlTil, output channel tile.
    IChnl, total input channel tile.
    OChnl, total output channel tile.
    Kern, kernel size
    WISec, total weight sectors in the weight buffer.
    ISec, total input sectors.
    OSec, total output sectors.
    Lyr, current conv layer.
    Til, current tile.
    ColNum, col number in current layer
    RowsRead, rows to be read in current cycle.
    RowsVlaid, rows to be valid.
    Rows, rows to be computed.
*/
void conv_ichnl(Dtype *In,
                Dtype *Param,
                Dtype BBuf[B_BUF_DEPTH],
                Dtype OutBuf[OTILE][O_BUF_DEPTH],
                int IChnlTil,
                int OChnlTil,
                int IChnl,
                int OChnl,
                int Kern,
                int WISec,
                int ISec,
                int OSec,
                int Lyr,
                int Til,
                int ColNum,
                int RowsRead,
                int RowsValid,
                int Rows)
{
  // Set on-chip buffer
  Dtype in_buf[ITILE][I_BUF_DEPTH];
  #pragma HLS array_partition variable=in_buf complete dim=1

  for (int n = 0; n < ISec; n++){ /* Input channel */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=4 max=4
    // Read input feature
    buf_read(In + (n << ITILSHFT) * ColNum, 
             in_buf, 
             IChnlTil,
             IChnl,
             n,
             0 == Til ? 1 : 2, // rows pre-read
             RowsRead,
             RowsValid,
             ColNum+2); 
    #ifdef CHECK_CPU
    //std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
    //             ": Check InBuf." << std::endl;
    inbuf_check(In + (n << ITILSHFT) * ColNum, 
                in_buf, 
                Lyr, 
                0 == Til ? 1 : 2, 
                RowsRead, 
                RowsValid);
    #endif

    conv_ochnl(Param, in_buf, BBuf, OutBuf,
               IChnlTil,
               OTILE,
               OChnl,
               Kern,
               n,
               WISec,
               OSec,
               Til,
               Lyr,
               Rows,
               ColNum + 2);
  }

  return;
}

/*
  Conv entry point in output channel.

  Arguments:

    Param, parameters.
    InBuf, input data buffer.
    BBuf, bias buffer.
    OutBuf, output buffer.
    IChnlTil, input channel tile.
    OChnlTil, output channel tile.
    OChnl, total output channel.
    Kern, kernel size.
    Ni, current input sector.
    WISec, weight sectors in the weight buffer.
    OSec, total output sector.
    Til, current tile number.
    Lyr, current layer number.
    RowNum, current rows to be compute.
    ColNum, current cols.
*/
void conv_ochnl(Dtype *Param,
                Dtype InBuf[ITILE][I_BUF_DEPTH],
                Dtype BBuf[B_BUF_DEPTH],
                Dtype OutBuf[OTILE][O_BUF_DEPTH],
                int IChnlTil,
                int OChnlTil,
                int OChnl,
                int Kern,
                int Ni,
                int WISec,
                int OSec,
                int Til,
                int Lyr,
                int RowNum,
                int ColNum)
{
  static Dtype w_buf[OTILE * ITILE][W_BUF_DEPTH];
  #pragma HLS array_partition variable=w_buf complete dim=1

  for(int m = 0; m < OSec; m++){ /* Output channel */
  //#pragma HLS DATAFLOW
  #pragma HLS loop_tripcount min=4 max=4
    // Conditional read weights 
    if (0 == Til || (Lyr > 12))
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
    if ((0 == Til)){
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
            RowNum,
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
  
  Arguments:

    Param, parameter.
    Wbuf, weight buffer.
    IChnlTil, input channel tile.
    OChnlTil, output channel tile.
    Kern, kernel number.
    Sec, which sector to read weight into.
    Read, whther read.
  
  Note:
  
    - In this design, only a sector (like 3 * 32 * 9)
    data was read into w_buf each cycles.
    - In some layer, w_buf can accomodate all
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

    InBuf, input data buffer.
    WBuf,  weights buffer.
    BBuf,  bias buffer.
    OutBuf, output data buffer.
    RowNum, rows number to be compute.
    ColNum, total col number in current layer.
    Kern, kernel size.
    IChnlTil, input channel tile.
    OChnlTil, output channel tile.
    OTilNum, total output channel tiles.
    ISec, input channel sector.
    OSec, output channel sector.
    LoadBias, whether to load bias.

  Note:

    - Current design is for k * k kernel size,
    not for k1 * k2 kernel.
    - Current design is only for conv stride 1,
    codes will be needed to modify for other stride size. 

*/
void compute(Dtype InBuf[ITILE][I_BUF_DEPTH],
             Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
             Dtype BBuf[B_BUF_DEPTH],
             Dtype OutBuf[OTILE][O_BUF_DEPTH],
             int RowNum,
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
  
  int rows_valid = RowNum - Kern + 1;
  int cols_valid = ColNum - Kern + 1;
  for (int row = 0; row < rows_valid; row++){ /* Row */
    for (int col = 0; col < cols_valid; col++) { /* Col */
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
             if(LoadBias && 0 == k1 && 0 == k2)  
               partial[m] = BBuf[OSec * OTILE + m];
             else
               partial[m] = 0.0;
             for (int n = 0; n < ITILE; n++) { /* In channel */
             //#pragma HLS UNROLL
               Dtype input = 0.0;
               if (n < IChnlTil)
                 input = InBuf[n][(row + k1) * ColNum + col + k2];

               Dtype weight = 0.0;
               if (n < IChnlTil)
                 weight = WBuf[m * ITILE + n]
                              [ISec * OTilNum * Kern * Kern + 
                               OSec * Kern * Kern + 
                               k1 * Kern + k2];
               
               partial[m] += weight * input;

             } /* In channel */

             if(LoadBias && 0 == k1 && 0 == k2)
             {
               OutBuf[m][OSec * rows_valid * cols_valid + row * cols_valid + col] = 
                 partial[m];
             }
             else {
               OutBuf[m][OSec * rows_valid * cols_valid + row * cols_valid + col] += 
                 partial[m];
             }
         
           } /* Out channel */

        } /* Kernl col */ 
      } /* Kernel row */ 
    } /* Col */
  } /* Row */
  return;
}
/*
  Write buf data to extern mem

  Arguments:

    OutBuf, output buffer.
    Out, external output mem.
    Pool, whether to pool.
    RowNum, number of row to be write off.
    ColNum, number of col to be write off.
    OSec,  output channel sector.

  Note:
  
    - This function contrain relu and max pooling op(stride 2). 
*/
void buf_write(Dtype OutBuf[OTILE][O_BUF_DEPTH], 
               Dtype *Out, 
               bool Pool,
               int RowNum,
               int ColNum,
               int OSec)
{
  int row_strd = Pool ? 2 : 1;
  for (int row = 0; row < RowNum; row+=row_strd){
    for (int m = 0; m < OSec; m++){
      for (int m_i = 0; m_i < OTILE; m_i++){
        for (int col = 0; col < ColNum; col+=2) {
        #pragma HLS PIPELINE
          Dtype data00 = OutBuf[m_i][m * RowNum * ColNum + row * ColNum + col];
          Dtype data01 = OutBuf[m_i][m * RowNum * ColNum + row * ColNum + col + 1];
          if(Pool){
            Dtype data10 = OutBuf[m_i][m * RowNum * ColNum + (row+1) * ColNum + col];
            Dtype data11 = OutBuf[m_i][m * RowNum * ColNum + (row+1) * ColNum + col + 1];
            Dtype data0_max = data00 > data01 ? data00 : data01;
            Dtype data1_max = data10 > data11 ? data10 : data11;
            Dtype data_max = data0_max > data1_max ? data0_max : data1_max;
            *Out++ = (data_max < 0) ? (Dtype)0.0 : data_max;
          }
          else{
            *Out++ = (data00 < 0) ? (Dtype)0.0 : data00;
            *Out++ = (data01 < 0) ? (Dtype)0.0 : data01;
          }
        } /* Col */  
      } /* OTile */
    } /* OSec */
  } /* Row */

  return;
}
