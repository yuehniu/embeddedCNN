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

// Conv in Xilinx FPGA
#pragma SDS data access_pattern(In:SEQUENTIAL, Out:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS, Out:PHYSICAL_CONTIGUOUS)
void conv_fpga(Dtype * In, Dtype * Out);

void conv_fpga(Dtype *In, Dtype *Params, Dtype *Out, 
               int Lyr, int TilNum, int ChnlTilNum);

// Read input
void buf_read(Dtype * In, Dtype InBuf[ITILE][FTILE_W * FTILE_H],
              int ChnlNum, int RowNum, int ColNum, 
              int RowShft, bool Fst);

// Move data
void buf_move(Dtype In_Buf[FTILE_W][FTILE_H], Dtype Out_Buf[FTILE_W][FTILE_H]);

// Write data
void buf_write(Dtype Out_Buf[FTILE_W][FTILE_H], Dtype *Out);

#endif /* __CONV_FPGA_H__ */
