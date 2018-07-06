/*
    Desc:
    
        FC Op for CNN in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        07/04/2018

    Author:

        Yue Niu
*/
#include "sds_lib.h"
#include "../../include/fpga/fc_fpga.h"
#include "../../include/common.h"
#include "../../include/utils/check.h"

/*
  Desc:
 
    Top module for fc in FPGA.

  Arguments:

    In, input data fron prev conv layer.
    Param, parameter.
    Out, fc output.

  Note:

    - This module consists of many fc layer ops.
*/
void fc_fpga(Dtype *In, Dtype *Param, Dtype *Out)
{
  // buffer for data
  Dtype bufferA[BUFA_DEPTH];
  Dtype bufferB[BUFB_DEPTH];

  int i_num, o_num = 0;
  int itils, otils, wtils = 0;
  for (int lyr = 0; lyr < FC_LAYER_NUM; lyr++){
    #ifdef CHECK_CPU_FC
    std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": " << lyr << "th FC layer." << std::endl;
    #endif
    switch(lyr){
      case 0:  {itils = 7; otils = 1; i_num = 25088; o_num = 256;       wtils = 64; break;}
      case 1:
      case 3:  {itils = 1; otils = 4; i_num = 256;   o_num = 1024;      wtils = 4;  break;}
      case 2:  {itils = 1; otils = 1; i_num = 4096;  o_num = 256;       wtils = 64; break;}
      case 4:  {itils = 1; otils = 1; i_num = 4096;  o_num = CLASS_NUM; wtils = 64; break;}
      default: {itils = 0; otils = 0; i_num = 0;     o_num = 0;         wtils = 0;  break;}
    }
  
    // FC in one layer
    fc_lyr(In, bufferA, Param, bufferB, 
           itils, otils, o_num, wtils, lyr);
    Param += o_num * otils + o_num * otils * i_num;

    #ifdef CHECK_CPU_FC
    if (4 == lyr)
      fc_check(bufferB, lyr);
    #endif
  }

  fc_buf_write(bufferB, Out);
  return;
}

/*
  FC in one layer.
*/
void fc_lyr(Dtype *In, Dtype BufferA[BUFA_DEPTH], 
            Dtype *Param, Dtype BufferB[BUFB_DEPTH], 
            int ITils, int OTils, int ONum, int WTils, int Lyr)
{
  // bias and weight buffer
  // Dtype b_buf[BUFA_DEPTH];

  Dtype w_buf[64][1024];
  #pragma HLS array_partition variable=w_buf complete dim=1

  // read bias first
  bool relu;
  if (Lyr & 0x1){
    relu = true;
    fc_bias_read(Param, BufferA, ONum * OTils);
  }
  else {
    relu = false;
    fc_bias_read(Param, BufferB, ONum * OTils);
  }
  #ifdef CHECK_CPU_FC
  fc_bias_check(Param, out_buf, ONum * OTils);
  #endif
  Param += ONum * OTils;

  for (int itil = 0; itil < ITils; itil++){
    if (0 == Lyr){
      int read_len = itil == ITils-1 ? 512 : BUFA_DEPTH;
      fc_buf_read(In + itil * read_len, BufferA, read_len);

      #ifdef CHECK_CPU_FC
      fc_inbuf_check(In + itil * read_len, BufferA, read_len);
      #endif
    }

    int wtils = (0 == Lyr && itil == ITils-1) ? 8 : WTils;
    for (int otil = 0; otil < OTils; otil++){
      for (int wtil = 0; wtil < wtils; wtil++){
        fc_weight_read(Param + (itil << 12) * ONum + (otil<<16) * wtils + (ONum << 6) * wtil, w_buf, ONum); 
        #ifdef CHECK_CPU_FC
        //fc_weight_check(Param + (itil << 12) * ONum + (otil<<17) * wtils + (ONum << 7) * wtil, w_buf, ONum);
        #endif
        fc_compute(BufferA, BufferB, w_buf, 
                   otil, ONum, wtil, Lyr, relu,
                  itil == ITils-1 && wtil == wtils-1);
      }
    }
  }

  return;
}

/*
  Read bias.
*/
void fc_bias_read(Dtype *Param, Dtype* BBuf, int ONum)
{
  for (int m = 0; m < ONum; m++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=256 max=4096
    BBuf[m] = *Param++;
  }
  return;
}

/*
  Read input data from prev conv layer.
*/
void fc_buf_read(Dtype *In, Dtype Buffer[BUFA_DEPTH], int Len)
{
  for (int n = 0; n < Len; n++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=512 max=4096
    Buffer[n] = *In++;
  }
  return;
}

/*
  Read weight.
*/
void fc_weight_read(Dtype *Param, Dtype WBuf[64][1024], int ONum)
{
  for (int m = 0; m < ONum; m++){
    for (int n = 0; n < 64; n++){
    #pragma HLS PIPELINE
    #pragma HLS loop_tripcount min=128000 max=131072
      WBuf[n][m] = *Param++;
    } 
  }
  return;
}

/*
  Parallel compute
*/
void fc_compute(Dtype BufferA[BUFA_DEPTH], 
                Dtype BufferB[BUFB_DEPTH], 
                Dtype WBuf[64][1024], 
                int OTil, int ONum, int WTil, 
                int Lyr, bool Relu, bool Last)
{
  Dtype part[64];
  #pragma HLS array_partition variable=part complete dim=1

  Dtype *in_buf, *out_buf;
  if (Lyr & 0x1){
    in_buf = BufferB;
    out_buf = BufferA; 
  }
  else {
    in_buf = BufferA;
    out_buf = BufferB; 
  }

  for (int n = 0; n < 64; n++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=64 max=64
    part[n] = in_buf[WTil * 64 + n];
  }

  for (int m = 0; m < ONum; m++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=1000 max=1024
    Dtype result = (Dtype)0.0;
    for(int n = 0; n < 64; n++){
      Dtype muls = part[n] * WBuf[n][m];
      result += muls;
    }
    Dtype partial = out_buf[OTil * 1024 + m];
    partial += result;
    out_buf[OTil * 1024 + m] = (Relu && Last) ? (partial < 0 ? (Dtype)0.0 : partial) : partial;
  }
  return;
}

/*
  write final off.
*/
void fc_buf_write(Dtype Buffer[BUFB_DEPTH], Dtype *Out)
{
  for (int m = 0; m < CLASS_NUM; m++){
  #pragma HLS PIPELINE
  #pragma HLS loop_tripcount min=1000 max=1000
    *Out++ = Buffer[m];
  }
}
