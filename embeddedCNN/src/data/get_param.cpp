/*
    Desc:
        
       This file is a function set that handle reading parameters. 

    Date:
    
        06/08/2018

    Author:
 
        Yue Niu
*/
#include <fstream>

#include "../../include/data/get_param.h"

extern const int SHAPE[];
extern const int CHNEL[];
extern const int KERNL[];

void get_params(Dtype *Params, int ReadSize)
{
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
               ": Load params from file..." << std::endl;
               
  std::ifstream param_bin("./data/params.bin", std::ios::binary);
  char *in_buf = reinterpret_cast<char *>(Params);
  param_bin.read(in_buf, ReadSize);
  return;
}

/*
  Desc:
    Computing required memory size for all parameters in CNN,
    including all weights and bias.

  Argument:

  Note: 
    This function only take input external variables.
*/
int param_size()
{
  int size = CHNEL[0] * 3 * KERNL[0] * KERNL[0] + CHNEL[0];

  // Conv layer
  for (int c_layer = 1; c_layer < CONV_LAYER_NUM; c_layer++)
  {
    // Weights
    size += CHNEL[c_layer] * CHNEL[c_layer - 1] * KERNL[0] * KERNL[0];
    // Bias
    size += CHNEL[c_layer];
  }

  // FC layer
  size += CHNEL[CONV_LAYER_NUM - 1] * SHAPE[CONV_LAYER_NUM - 1] * 
          SHAPE[CONV_LAYER_NUM - 1] / 4 * CHNEL[CONV_LAYER_NUM];
  for (int f_layer = 1; f_layer < FC_LAYER_NUM; f_layer++)
  {
    // Weights
    size += CHNEL[CONV_LAYER_NUM + f_layer] * 
            CHNEL[CONV_LAYER_NUM + f_layer - 1];
    // Bias
    size += CHNEL[CONV_LAYER_NUM + f_layer];
  }
  return size;
}