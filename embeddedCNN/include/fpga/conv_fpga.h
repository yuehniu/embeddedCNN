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
#pragma SDS data access_pattern(In:SEQUENTIAL, Param:SEQUENTIAL, Out:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS, Param:PHYSICAL_CONTIGUOUS, Out:PHYSICAL_CONTIGUOUS)
#pragma SDS data copy(In[0:IChnl*RowNum*ColNum], Param[0:OChnl+IChnl*OChnl*Kern*Kern], Out[0:OChnl*RowNum*ColNum])
void conv_fpga(Dtype *In,    // Variable DMA transfer length 
               Dtype *Param, // Variable DMA transfer length
               Dtype *Out,   // Variable DMA transfer length
               int Lyr,      // Current layer index 
               int RowNum,   // Row number
               int ColNum,   // Col number
               int ChnlRead, // Input channel to read in each tile
               int Kern,     // Kernel size
               int IChnl,    // Input param channel to read
               int ISec,     // Input sec number
               int OChnl,    // Output param channel to read
               int OSec,     // Out sec number
               int WISec     // Input sec number in buf for each layer
              );

// Read data into InBuf
void buf_read(Dtype * In, 
              Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H],
              int ChnlNum,
              int ColNum);

// Read weight
void weight_read(Dtype *Param, 
                 Dtype Wbuf[OTILE * ITILE][W_BUF_DEPTH],
                 int IChnlTil,
                 int OChnlTil,
                 int Kern,
                 int Sec,
                 bool Read);
// Read bias
void bias_read(Dtype *Param, Dtype Bbuf[B_BUF_DEPTH], int OChnl);

// Parallel computing unit
void compute(Dtype InBuf[ITILE][(FTILE_W+2) * FTILE_H],
             Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
             Dtype BBuf[B_BUF_DEPTH],
             Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC ],
             int Row,
             int ColNum,
             int Kern,
             int IChnlTil,
             int OChnlTil,
             int OTilNum,
             int ISec,
             int OSec,
             bool LoadBias);

// Write data
// #pragma SDS data mem_attribute(Out:PHYSICAL_CONTIGUOUS)
// #pragma SDS data access_pattern(Out_Buf:SEQUENTIAL, Out:SEQUENTIAL)
void buf_write(Dtype OutBuf[OTILE][O_BUF_ROW * FTILE_W * FTILE_H * O_BUF_SEC], 
               Dtype *Out, 
               int Row,
               bool Write,
               int ColNum,
               int OSec);

#endif /* __CONV_FPGA_H__ */
