/*
    Desc:
    
        Function set for CNN in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        06/04/2018

    Author:

        Yue Niu
*/

#ifndef __CNN_FPGA_H__
#define __CNN_FPGA_H__

#include "../include/common.h"

#pragma SDS data access_pattern(In:SEQUENTIAL, Out:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS, Out:PHYSICAL_CONTIGUOUS)
void conv_fpga(Dtype * In, Dtype * Out);

// Read input
void buf_read(Dtype * In, Dtype In_Buf[TILE_SIZE][TILE_SIZE]);

// Move data
void buf_move(Dtype In_Buf[TILE_SIZE][TILE_SIZE], Dtype Out_Buf[TILE_SIZE][TILE_SIZE]);

// Write data
void buf_write(Dtype Out_Buf[TILE_SIZE][TILE_SIZE], Dtype *Out);

#endif /* __CNN_FPGA_H__ */
