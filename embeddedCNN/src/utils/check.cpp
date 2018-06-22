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
extern const int CHNEL[];

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
void inbuf_check(Dtype *Ref, 
                 Dtype InBuf[ITILE][(FTILE_W+2)*FTILE_H], 
                 int Lyr, 
                 int Til)
{
  
  std::ofstream log("check_InBuf.log", std::ios::app);
  int chnl_til_num = Lyr == 0 ? 3 : ITILE; 
  int col_num = SHAPE[Lyr] + 2;
  Dtype *ref_ptr = Ref;
  log << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
         ": Check " << Til << "th tile." << std::endl;
    for (int ch = 0; ch < chnl_til_num; ch++){
      for (int row = 0; row < FTILE_H; row++){
        for (int col = 0; col < col_num; col++){
          Dtype ref = 0.0;
          if ((0 == col) || (col_num-1 == col))
            ref = 0.0; 
          else
            ref = *(ref_ptr+ Til * chnl_til_num * FTILE_H * (col_num-2) + 
                          ch * FTILE_H * (col_num-2) +
                          row * (col_num-2) + col - 1);
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
    for(int ich = 0; ich < ITILE; ich++) {
      for(int k = 0; k < Kern * Kern; k++){
        if (ich < IChnlTil){
          Dtype ref = *(Param + 
                        och * IChnlTil * Kern * Kern + 
                        ich * Kern * Kern + k);
          Dtype wbuf = WBuf[och * ITILE + ich][Sec * Kern * Kern + k];
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

/* Check computing result */
void computing_check(Dtype *Out, int Lyr)
{
  std::ofstream log("check_conv_result.log", std::ios::app);
  std::ifstream feature("./data/conv1_1fp16.bin", std::ios::binary); 
  int r_size = CHNEL[Lyr] * SHAPE[Lyr] * SHAPE[Lyr];
  Dtype *ref_feat = (Dtype *) malloc(r_size * sizeof(Dtype));
  char *ref_char = reinterpret_cast<char *>(ref_feat); 
  feature.read(ref_char, r_size * sizeof(Dtype));

  for (int row = 0; row < SHAPE[Lyr]; row++) {
    for (int och = 0; och < CHNEL[Lyr]; och++) {
      for (int col = 0; col < SHAPE[Lyr]; col++) {
        int pos = row * CHNEL[Lyr] * SHAPE[Lyr] + och * SHAPE[Lyr] + col;
        Dtype ref = *(ref_feat + pos);
        Dtype out = *(Out + pos);
        if (ref < 15.0){
          float abs_err = ref - out;
          if (-ABS_ERR <= abs_err && abs_err <= ABS_ERR)
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << row << "th row, " << 
                   och << "th channel, " <<
                   col << "th col data check pass." <<
                   std::endl;
          else
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << row << "th row, " << 
                   och << "th channel, " <<
                   col << "th col data check fail." <<
                   std::endl;
        }
        else{
          float rel_err = (ref - out) / ref;
          if (-REL_ERR <= rel_err && rel_err <= REL_ERR)
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << row << "th row, " << 
                   och << "th channel, " <<
                   col << "th col data check pass." <<
                   std::endl;
          else
            log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
                   ": " << row << "th row, " << 
                   och << "th channel, " <<
                   col << "th col data check fail." <<
                   std::endl;
        }
        log << "[LOG] " << __FUNCTION__ << ", " << __LINE__ <<
               ": Ref data, " << ref << "; Compute data, " << out <<  
               std::endl; 
      }
    } 
  }

  free(ref_feat);
  return;
}
