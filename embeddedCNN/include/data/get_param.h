/*
    Desc:
        
       This file is a function set that handle parameters. 

    Date:
    
        06/04/2018

    Author:
 
        Yue Niu
*/
#include "../common.h"

#ifndef __GET_PARAM_H__
#define __GET_PARAM_H__

/* Computing param size for a model */
int param_size();

// Read parameter
void get_params(Dtype *Params, int ReadSize);

#endif
