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
#include "../include/cpu/cnn_cpu.h"
#include "../include/common.h"
#include "../include/data/get_data.h"
#include "../include/data/get_param.h"
#include "../include/utils/check.h"

int main(int argc, char* argv[])
{
  std::cout << "Embedded CNN implementation in ZCU102" << std::endl;
  // Get input data
  Dtype * in_rgbImg =
    (Dtype *) sds_alloc(3 * IMG_W * IMG_H * sizeof(Dtype));
  if (NULL == in_rgbImg){
    std::cout << "[Error] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": Memory alloc for input data fail" << std::endl;
    return -1;
  }
  memset(in_rgbImg, 0, 3 * IMG_W * IMG_H * sizeof(Dtype));
  get_img(in_rgbImg);

  // Alloc output memory
  Dtype * out_feat =
    (Dtype *) sds_alloc(3 * IMG_W * IMG_H * sizeof(Dtype));
  if (NULL == out_feat){
    std::cout << "[Error] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": Memory alloc for output fail" << std::endl;
    return -1;
  }
  memset(out_feat, 0, 3 * IMG_W * IMG_H * sizeof(Dtype));

  // Get parameters

  // CNN in ARM CPU

  // CNN in FPGA 
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
            ": Start conv in Xilinx FPGA." << std::endl;
  conv_fpga(in_rgbImg, out_feat);

  // Check dataflow
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
            ": Check dataflow between DDR and FPGA" << std::endl;
  bool all_same = dataflow_check(in_rgbImg, out_feat, 3 * IMG_W * IMG_H);
  if (true == all_same)
    std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__  <<
                 ": Dataflow check pass." << std::endl;
  else
    std::cout << "[ERROR] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": Dataflow check fail." << std::endl;

  return 0;
}
