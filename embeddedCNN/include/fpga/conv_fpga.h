/*
    Desc:
    
        Conv Op for CNN in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        06/04/2018

    Author:

        Yue Niu
*/

#ifndef __CONV_FPGA_H__
#define __CONV_FPGA_H__

#include "../include/common.h"

#pragma SDS data access_pattern(In:SEQUENTIAL, Out:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS, Out:PHYSICAL_CONTIGUOUS)
void conv_fpga(Dtype * In, Dtype * Out);

void conv_fpga(Dtype *In, Dtype *Params, Dtype *Out);

// Read input
void buf_read(Dtype * In, Dtype In_Buf[FTILE][FTILE]);

// Move data
void buf_move(Dtype In_Buf[FTILE][FTILE], Dtype Out_Buf[FTILE][FTILE]);

// Write data
void buf_write(Dtype Out_Buf[FTILE][FTILE], Dtype *Out);

#endif /* __CONV_FPGA_H__ */
