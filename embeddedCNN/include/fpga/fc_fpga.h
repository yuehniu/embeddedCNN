/*
    Desc:
    
        FC Op for CNN in Xilinx FPGA.

    Note:
        
        Platform: ZCU102
        Toolkit: SDx 2018.1

    Date:

        07/04/2018

    Author:

        Yue Niu
*/

#include "../../include/common.h"

/* Top module for fc in FPGA */
#pragma SDS data access_pattern(In:SEQUENTIAL, Param:SEQUENTIAL, Out:SEQUENTIAL)
#pragma SDS data mem_attribute(In:PHYSICAL_CONTIGUOUS, Param:PHYSICAL_CONTIGUOUS, Out:PHYSICAL_CONTIGUOUS)
#pragma SDS data copy(In[0:I_LENGTH], Param[0:P_LENGTH], Out[0:CLASS_NUM])
void fc_fpga(Dtype *In, Dtype *Param, Dtype *Out);

// FC for one layer
void fc_lyr(Dtype *In, Dtype BufferA[BUFA_DEPTH], 
            Dtype *Param, Dtype BufferB[BUFB_DEPTH], 
            int ITils, int OTils, int ONum, int WTils, int Lyr);

// Read bias
void fc_bias_read(Dtype *Param, Dtype* BBuf, int ONum);

// Read input data from prev layer
void fc_buf_read(Dtype *In, Dtype Buffer[BUFA_DEPTH], int Len);

// Read weight
void fc_weight_read(Dtype *Param, Dtype WBuf[64][1024], int ONum);

// Compute
void fc_compute(Dtype *In, 
                Dtype *Out, 
                Dtype WBuf[64][1024], 
                int OTil, int ONum, int WTil, 
                int Lyr, bool Relu,  bool Last);

// write output off
void fc_buf_write(Dtype Buffer[BUFB_DEPTH], Dtype *Out);
