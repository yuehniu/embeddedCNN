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
            log << "[ERR] " << __FUNCTION__ << ", " << __LINE__ <<
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

/* Check w_buf */
void wbuf_check(Dtype *Param, 
                Dtype WBuf[OTILE * ITILE][W_BUF_DEPTH],
                int IChnlTil,
                int OChnlTil,
                int Kern,
                int Sec
                )
{
  std::ofstream log("check_WBuf.log", std::ios::app);
  log << "[INFO] " << __FUNCTION__ << ", " << __LINE__ <<
         ": Check " << Sec << "th sector." << std::endl;  

  for(int och = 0; och < OChnlTil; och++){
    for(int ich = 0; ich < IChnlTil; ich++) {
      for(int k = 0; k < Kern * Kern; k++){
        Dtype ref = *(Param + 
                      och * IChnlTil * Kern * Kern + 
                      ich * Kern * Kern + k);
        Dtype wbuf = WBuf[och * IChnlTil + ich][Sec * Kern * Kern + k];
        if (ref == wbuf){
          log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": " << Sec * OChnlTil + och << "th ochannel, " <<
                         ich << "th ichannel, " <<
                         k   << "th weight check pass." << 
                         std::endl;
        }
        else{
          log << "[ERR] " << __FUNCTION__ << ", " << __LINE__ <<
                 ": " << Sec << "th sector, " <<
                         och << "th ochannel, " <<
                         ich << "th ichannel, " <<
                         k   << "th weight check fail." << 
                         std::endl;
        }
        log << "[LOG] " << "Ref weight: " << ref <<
               ", WBuf: " << wbuf << std::endl;
      }
    }
  }

  log.close();
  return;
}

/* Check b_buf */
void bbuf_check(Dtype *Param, Dtype BBuf[B_BUF_DEPTH], int OChnl)
{
  std::ofstream log("check_BBuf.log", std::ios::app);

  for (int och = 0; och < OChnl; och++){
    Dtype ref = *(Param + och);
    Dtype bbuf = BBuf[och];
    if (ref == bbuf){
      log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << och << "th ochannel bias check pass." <<
             std::endl;
    }
    else {
      log << "[ERR] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << och << "th ochannel bias check fail." <<
             std::endl;
    }
    log << "[LOG] " << "Ref bias: " << ref <<
           ", BBuf: " << bbuf << std::endl;
  }

  log.close();
  return;
}

/* Check b_buf */
void onchip_check(Dtype *Ref, Dtype *Chip, int OChnl)
{
  std::ofstream log("check_Onchip.log", std::ios::app);

  for (int och = 0; och < OChnl; och++){
    Dtype ref = *(Ref + och);
    Dtype bbuf = *(Chip + och);
    if (ref == bbuf){
      log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << och << "th ochannel bias check pass." <<
             std::endl;
    }
    else {
      log << "[ERR] " << __FUNCTION__ << ", " << __LINE__ <<
             ": " << och << "th ochannel bias check fail." <<
             std::endl;
    }
    log << "[LOG] " << "Ref bias: " << ref <<
           ", BBuf: " << bbuf << std::endl;
  }

  log.close();
  return;
}
