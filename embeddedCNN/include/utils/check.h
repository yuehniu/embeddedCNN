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
void inbuf_check(Dtype *Ref, Dtype InBuf[ITILE][FTILE_W*FTILE_H], int Lyr, int Til);

#endif
