/*
    Desc:

        This is a main functin for embedded cnn implementation.
        To check the result from FPGA, we realize cnn both in ARM CPU 
        and Xilinx FPGA.
        At the begining, we first realize VGG16, since it only contains
        3x3 kernels.
        Then, a generalized design will be considered.

    Date:
        
        06/04/2018
   
    Author:
        
        Yue Niu
*/

#include <iostream>
#include <fstream>
#include <string.h>

#include "sds_lib.h"

#include "../include/fpga/cnn_fpga.h"
#include "../include/fpga/conv_fpga.h"
#include "../include/cpu/cnn_cpu.h"
#include "../include/common.h"
#include "../include/data/get_data.h"
#include "../include/data/get_param.h"
#include "../include/utils/check.h"

int main(int argc, char* argv[])
{
  std::cout << "Embedded CNN implementation in Xilinx FPGA" << std::endl;
  std::cout << "Platform: Xilinx ZCU102" << std::endl;
  std::cout << "CPU: ARM A53" << std::endl;
  std::cout << "NetModel: VGGNet-16" << std::endl;
  
  // Get input data
  Dtype * in_rgbImg =
    (Dtype *) sds_alloc(3 * IMG_W * IMG_H * sizeof(Dtype));
  mem_check(in_rgbImg);
  memset(in_rgbImg, 0, 3 * IMG_W * IMG_H * sizeof(Dtype));
  get_img(in_rgbImg);

  // Alloc output memory
  Dtype * out_logits =
    (Dtype *) sds_alloc(CLASS_NUM * sizeof(Dtype));
  mem_check(out_logits);
  memset(out_logits, 0, CLASS_NUM * sizeof(Dtype));

  // Get parameters
  // Set memory for params, including all weights and bias.
  int param_buf_size = param_size();
  Dtype *params = (Dtype *)sds_alloc(param_buf_size * sizeof(Dtype));
  mem_check(params);
  //get_params(params, param_buf_size);

  // CNN in ARM CPU

  // CNN in FPGA 
  cnn_fpga(in_rgbImg, out_logits, params);

  // Check dataflow
  sds_free(in_rgbImg); sds_free(out_logits); sds_free(params);
  return 0;
}
