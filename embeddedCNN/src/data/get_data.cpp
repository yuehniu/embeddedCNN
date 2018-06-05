/*
    Desc:
        
       This file is a function set that handle input data. 

    Date:
    
        06/04/2018

    Author:
 
        Yue Niu
*/

#include <stdlib.h>
#include <stdint.h>

#include "../../include/data/get_data.h"
#include "sds_lib.h"

void get_img(Dtype * Img)
{
  int im_size = IMG_H * IMG_W;
  for (int c = 0; c < 3; c++){
    for (int row = 0; row < IMG_H; row++){
      for (int col = 0; col < IMG_W; col++){
        Img[c*im_size + row * IMG_W + col] = 
          rand() / (IMG_W * IMG_H);
      }
    }
  }

  return;
}
