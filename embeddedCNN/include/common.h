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

/* Tile params */
// Input channel tile size
#define ITILE 16
// Output channel title size
#define OTILE 32
// Feature map title size
#define FTILE 21

/* Number of Tile loops */
#define TILE_NUM (3 * (IMG_W / FTILE) * (IMG_H / FTILE)) 

#endif /* __COMMON_H__ */
