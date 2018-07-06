/*
    Desc:
    
        Result check function set.
        * Accurate data check (Dataflow).
        * Approximate data check (Compute result).

    Date:

        06/05/2018

    Author:
    
        Yue Niu
*/

#ifndef __CHECK_H__
#define __CHECK_H__

#include <stdlib.h>
#include <iostream>

#include "../../include/common.h"

/* check on ARM CPU? */
//#define CHECK_CPU_CONV
//#define CHECK_FPGA_CONV
//#define CHECK_CPU_FC
#define CHECK_FPGA_FC

#define REL_ERR 0.03
#define ABS_ERR 2

/* Accurate data check */
bool dataflow_check(Dtype * Ref, Dtype * Res, int Cnt);

/* Bad memory check */
inline void mem_check(Dtype *Mem)
{
  if (NULL == Mem){
    std::cout << "[ERROR] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": Memory check fail" << std::endl;
    exit(1);
  }
  return;
}

/* Check in_buf */
void conv_inbuf_check(
  Dtype *Ref, 
  Dtype InBuf[ITILE][I_BUF_DEPTH], 
  int Lyr, 
  int RowsPre,
  int RowsRead,
  int RowsValid
);

/* Check w_buf */
void conv_wbuf_check(
  Dtype *Param, 
  Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
  int IChnlTil,
  int OChnlTil,
  int Kern,
  int Sec
);

/* Check b_buf */
void conv_bias_check(Dtype *Param, Dtype BBuf[B_BUF_DEPTH], int OChnl);

/* Check 0n-chip data */
void onchip_check(Dtype *Ref, Dtype *Chip, int OChnl);

/* Check computing result */
void conv_check(Dtype *Out, int Lyr, bool Pooling);

/* Check output from FC */
void fc_check(Dtype *Out, int Lyr);

void fc_bias_check(Dtype *Param, Dtype *BBuf, int Len);

void fc_inbuf_check(Dtype *In, Dtype BufferA[BUFA_DEPTH], int Len);

void fc_weight_check(Dtype *Param, Dtype WBuf[128][1024], int ONum);
#endif
