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

// Data type
typedef float Dtype;

// Feature params
#define IMG_W 224
#define IMG_H 224

// Tile params
#define TILE_SIZE 14

// Number of Tile loops
#define TILE_NUM (3 * (IMG_W / TILE_SIZE) * (IMG_H / TILE_SIZE)) 

#endif /* __COMMON_H__ */
