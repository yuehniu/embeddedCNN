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

#include "../../include/common.h"

// Conv in Xilinx FPGA
#pragma SDS data access_pattern(In:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS)
#pragma SDS data copy(In[0:TilNum*ChnlRead*ColNum])
void conv_fpga(Dtype *In, // Variable DMA transfer length 
               int Lyr, 
               int TilNum, 
               int ChnlRead, 
               int ColNum);

// Read data into InBuf
void buf_read(Dtype * In, 
              Dtype InBuf[ITILE][FTILE_W * FTILE_H],
              int ChnlNum,
              int ColNum);
// Write data
// #pragma SDS data mem_attribute(Out:PHYSICAL_CONTIGUOUS)
// #pragma SDS data access_pattern(Out_Buf:SEQUENTIAL, Out:SEQUENTIAL)
// void buf_write(Dtype Out_Buf[FTILE_W][FTILE_H], Dtype *Out);

#endif /* __CONV_FPGA_H__ */
