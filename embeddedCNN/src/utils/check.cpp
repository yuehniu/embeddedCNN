/*
    Desc:
    
        Result check function set.
        * Accurate data check (Dataflow).
        * Approximate data check (Compute result).

    Date:

        06/05/2018

    Author:
    
        Yue Niu
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "../../include/utils/check.h"


extern const int SHAPE[];

bool dataflow_check(Dtype * Ref, Dtype * Res, int Cnt)
{
  std::cout << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
            ": Check dataflow between DDR and FPGA" << std::endl;
            
  bool all_same = true;
  std::ofstream log("check_df.log");
  for (int i = 0; i < Cnt; i++){
    if (*(Res + i) != *(Ref + i))
    {
      all_same = false;
      log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << i << "th data check fail" << std::endl;
      log << "[LOG] " << "Ref data: " << *(Ref + i) <<
             ", Result data: " << *(Res + i) << std::endl;
    }
    else {
      log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << i << "th data check pass" << std::endl;
      log << "[LOG] " << "Ref data: " << *(Ref + i) <<
             ", Result data: " << *(Res + i) << std::endl;
    }
  }
  log.close();

  return all_same;
}

/* Check in_buf */
void inbuf_check(Dtype *Ref, Dtype InBuf[ITILE][FTILE_W*FTILE_H], int Lyr, int Til)
{
  
  std::ofstream log("check_InBuf.log", std::ios::app);
  int chnl_til_num = Lyr == 0 ? 3 : ITILE; 
  int col_num = SHAPE[Lyr];
  Dtype *ref_ptr = Ref;
  log << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
         ": Check " << Til << "th tile." << std::endl;
    for (int ch = 0; ch < chnl_til_num; ch++){
      for (int row = 0; row < FTILE_H; row++){
        for (int col = 0; col < col_num; col++){
          Dtype ref = *(ref_ptr+ Til * chnl_til_num * FTILE_H * col_num + 
                        ch * FTILE_H * col_num +
                        row * col_num + col);
          Dtype inbuf = InBuf[ch][row * col_num + col];
          if (ref == inbuf)
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << ch << "th channel, " <<
                           row << "th row, " <<
                           col << "th col data check pass." <<
                           std::endl;
          else
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << ch << "th channel, " <<
                           row << "th row, " <<
                           col << "th col data check fail." <<
                           std::endl;
          
          log << "[LOG] " << "Ref data: " << ref << 
                 ", InBuf: " << inbuf << std::endl;
        }
      }
    }
  
  log.close();
  return;
}

