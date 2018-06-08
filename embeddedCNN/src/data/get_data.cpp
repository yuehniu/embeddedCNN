/*
    Desc:
        
       This file is a function set that handle input data. 

    Date:
    
        06/04/2018

    Author:
 
        Yue Niu
*/

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>

#include "../../include/data/get_data.h"
#include "sds_lib.h"

// Randomly init image
void get_from_random(Dtype * Img)
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

// Get image from file
void get_from_file(Dtype * Img)
{
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
               ": Read image from file." << std::endl;
  std::ifstream img_bin("./data/img.bin", std::ios::binary);
  char * in_buf = reinterpret_cast<char *>(Img);
  img_bin.read(in_buf, 3 * IMG_W * IMG_H * sizeof(Dtype));
  return;
}
