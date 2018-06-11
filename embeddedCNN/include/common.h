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

/* Data type */
typedef half Dtype;

/* Network definition for VGG16 */
#define CLASS_NUM 1000
#define CONV_LAYER_NUM 13
#define FC_LAYER_NUM 5
#define IMG_W 224
#define IMG_H 224

/* Conv params */
// Input channel tile size
#define ITILE 16
// Output channel title size
#define OTILE 16
// Feature map title size(3x224)
// This set is suitable for input with 224x224,
// kernel size with 3x3
#define FTILE_W 224
#define FTILE_H 3
// Define buffer depth for weights
// Note the buffer depth is closely related to kernel size,
// input and output kernel number
#define W_BUF_DEPTH 288
// Define buffer depth for output
// Note this buffer depth is also closely related to model,
// mainly including maximum output channel number.
#define O_BUF_DEPTH 512

/* Number of Tile loops */
#define TILE_NUM (3 * (IMG_W / FTILE) * (IMG_H / FTILE)) 

#endif /* __COMMON_H__ */
