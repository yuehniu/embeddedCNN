/*
    Desc:
    
        CNN top in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        06/08/2018

    Author:

        Yue Niu
*/

#ifndef __CNN_FPGA__
#define __CNN_FPGA__

#include "../include/common.h"
#include "../../include/fpga/cnn_fpga.h"

/* Top cnn module in Xilinx FPGA platform */
void cnn_fpga(Dtype *In, Dtype *Out, Dtype *Params);

#endif
