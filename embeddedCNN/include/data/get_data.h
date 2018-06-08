/*
    Desc:
        
       This file is a function set that handle input data. 

    Date:
    
        06/04/2018

    Author:
 
        Yue Niu
*/

#ifndef __GET_DATA_H__
#define __GET_DATA_H__ 

#include "../include/common.h"

// Randomly init image
void get_from_random(Dtype * Img);

// Get image from file
void get_from_file(Dtype * Img);

// Specify one data fetching method
//void (*get_img)(Dtype * ) = get_img_random;
#define get_img get_from_file

#endif
