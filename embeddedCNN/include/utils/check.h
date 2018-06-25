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

#define REL_ERR 0.05
#define ABS_ERR 1

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
void inbuf_check(Dtype *Ref, 
                 Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H], 
                 int Lyr, 
                 int Til);

/* Check w_buf */
void wbuf_check(Dtype *Param, 
                Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
                int IChnlTil,
                int OChnlTil,
                int Kern,
                int Sec
                );

/* Check b_buf */
void bbuf_check(Dtype *Param, Dtype BBuf[B_BUF_DEPTH], int OChnl);

/* Check 0n-chip data */
void onchip_check(Dtype *Ref, Dtype *Chip, int OChnl);

/* Check computing result */
void computing_check(Dtype *Out, int Lyr, bool Pooling);
#endif
