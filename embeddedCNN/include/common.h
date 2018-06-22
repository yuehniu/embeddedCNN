/*
    Desc:
    
        This file contains several pre-define parameter for the system.

    Date:

        06/04/2018

    Author:

        Yue Niu
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include "hls_half.h"

/* check on ARM CPU? */
//#define CHECK_CPU
//#define CHECK_FPGA

/* Data type */
typedef half Dtype;

/* Network definition for VGG16 */
#define CLASS_NUM 1000
#define CONV_LAYER_NUM 1
#define FC_LAYER_NUM 5
#define IMG_W 224
#define IMG_H 224

/* Conv params */
// Input channel tile size
#define ITILE 16
// Output channel title size
#define OTILE 32
// Feature map title size(3x224)
// This set is suitable for input with 224x224,
// kernel size with 3x3
#define FTILE_W 224
#define FTILE_H 1
// Define buffer depth for weights and bias
// Note the buffer depth is closely related to kernel size,
// input and output kernel number
#define W_BUF_DEPTH 288
#define B_BUF_DEPTH 512
// Define buffer depth for output
// Note this buffer depth is also closely related to model,
// mainly including maximum output channel number.
#define O_BUF_CHNL 512
#define O_BUF_SEC 16 // 512/32
// Define the number of rows buffered in output buffer.
// This number is closely related to kernel size and strides.
#define O_BUF_ROW 3

/* Conv unit interface */
#define FTRANS_SIZE (3 * 224 * 224)
#define PTRANS_SIZE (512 * 512 * 3 * 3)

#endif /* __COMMON_H__ */
