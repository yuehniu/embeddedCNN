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
#include <iostream>

#include "sds_lib.h"

#include "../../include/fpga/cnn_fpga.h"
#include "../../include/fpga/conv_fpga.h"
#include "../../include/data/get_param.h"
#include "../include/utils/check.h"
#include "../include/common.h"

extern const int SHAPE[];
extern const int CHNEL[];
extern const int KERNL[];

/* 
  Desc:

    Top cnn module in Xilinx FPGA platform.
    This module takes input RGB image, outputing 1000 classification 
    results.

  Argument:

    In, pointer to input RGB data.
    Out, pointer to output classification result.

  Note:

*/
void cnn_fpga(Dtype *In, Dtype *Out, Dtype *Params)
{
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
               ": Start CNN in Xilinx FPGA..." << std::endl;

  // Set two ping-pang memory for layer input and output
  // buf size should meet maximum memory requirement for
  // each layer in CNN
  int max_buf_size = SHAPE[0] * SHAPE[0] * CHNEL[0];
  Dtype *bufferA = (Dtype *)sds_alloc(max_buf_size * sizeof(Dtype));
  Dtype *bufferB = (Dtype *)sds_alloc(max_buf_size * sizeof(Dtype));
  mem_check(bufferA); mem_check(bufferB);

  /* Do conv layer by layer */
  // Set a pingpang memory flag
  // 0: bufferA/In(input), bufferB(output)
  // 1: bufferA(output), bufferB(input)
  int pingpang = 0;
  Dtype *cur_params = Params;
  for (int c_layer = 0; c_layer < CONV_LAYER_NUM; c_layer++)
  {
    if (0 == c_layer){
      std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                  ": " << c_layer << "th convolution layer." << std::endl;
      conv_fpga(In, bufferB, cur_params);
      cur_params += (CHNEL[0] * 3 * KERNL[0] * KERNL[0] + CHNEL[0]);
      pingpang = 1;
    }
    std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": " << c_layer << "th convolution layer." << std::endl;
    if (0 == pingpang)
    {
      conv_fpga(bufferA, cur_params, bufferB);
      pingpang = 1;
    }
    else 
    {
      conv_fpga(bufferB, cur_params, bufferA);
      pingpang = 0;
    }

    // Update pointer to parameters
    cur_params += (CHNEL[c_layer] * CHNEL[c_layer - 1] * KERNL[0] * KERNL[0] +
                   CHNEL[c_layer]);
  }

  /* Do FC layer by layer */
  for (int f_layer = 0; f_layer < FC_LAYER_NUM; f_layer++)
  {
    std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": " << f_layer << "th FC layer." << std::endl;
    //fc_fpga();
  }

  // Free memory
  sds_free(bufferA); sds_free(bufferB);
  return;
}
