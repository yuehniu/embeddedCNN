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

#include "../../include/common.h"

bool dataflow_check(Dtype * Ref, Dtype * Res, int Cnt);


#endif
