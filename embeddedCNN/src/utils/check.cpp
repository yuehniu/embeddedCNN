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
                 Dtype InBuf[ITILE][I_BUF_DEPTH], 
                 int Lyr, 
                 int RowsPre,
                 int RowsRead,
                 int RowsValid)
{
  static int til;
  if (1 == RowsPre) til = 0;
  else              til += 1;
  std::ofstream log("check_InBuf.log", std::ios::app);
  int chnl_til_num = Lyr == 0 ? 3 : ITILE; 
  int chnl_num = Lyr == 0 ? 3 : CHNEL[Lyr - 1];
  int col_num = SHAPE[Lyr] + 2;
  Dtype *ref_ptr = Ref;
  log << "[INFO] " << __FUNCTION__ << ", " << __LINE__ << 
         ": Check " << til << "th tile." << std::endl;
    for (int ch = 0; ch < chnl_til_num; ch++){
      for (int row = 0; row < RowsPre+RowsValid; row++){
        for (int col = 0; col < col_num; col++){
          Dtype ref = 0.0;
          if ((0 == col) || (col_num-1 == col))
            ref = 0.0; 
          else {
            if (0 == row){
              if (1 == RowsPre) 
                ref = 0.0;
              else 
                ref = *(ref_ptr - 2 * chnl_num * (col_num-2) + ch * (col_num-2) + col - 1);
            }
            else if (1 == row){
              if (1 == RowsPre) 
                ref = *(ref_ptr + ch * (col_num-2) + col - 1);
              else
                ref = *(ref_ptr - chnl_num * (col_num-2) + ch * (col_num-2) + col - 1);
            }
            else {
                ref = *(ref_ptr + (row - RowsPre) * chnl_num * (col_num-2) + ch * (col_num-2) + col - 1);
            }
          }
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
                   ": " << Sec << "th sector, " <<
                           och << "th ochannel, " <<
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
void computing_check(Dtype *Out, int Lyr, bool Pooling)
{
  std::ofstream log("check_conv_result.log", std::ios::app);
  std::ifstream feature;
  if (0 == Lyr)
    feature.open("./data/conv1_1fp16.bin", std::ios::binary); 
  else if (1 == Lyr){
    if (Pooling)
      feature.open("./data/pool1fp16.bin", std::ios::binary); 
    else
      feature.open("./data/conv1_2fp16.bin", std::ios::binary); 
  }
  else if (2 == Lyr)
    feature.open("./data/conv2_1fp16.bin", std::ios::binary); 
  else if (3 == Lyr){
    if (Pooling)
      feature.open("./data/pool2fp16.bin", std::ios::binary); 
    else 
      feature.open("./data/conv2_2.bin", std::ios::binary); 
  }
  else if (4 == Lyr)
    feature.open("./data/conv3_1fp16.bin", std::ios::binary); 
  else if (5 == Lyr)
    feature.open("./data/conv3_2fp16.bin", std::ios::binary); 
  else if (6 == Lyr){
    if (Pooling)
      feature.open("./data/pool3fp16.bin", std::ios::binary); 
    else 
      feature.open("./data/conv3_3.bin", std::ios::binary); 
  }
  else if (7 == Lyr)
    feature.open("./data/conv4_1fp16.bin", std::ios::binary); 
  else if (8 == Lyr)
    feature.open("./data/conv4_2fp16.bin", std::ios::binary); 
  else if (9 == Lyr){
    if (Pooling)
      feature.open("./data/pool4fp16.bin", std::ios::binary); 
    else 
      feature.open("./data/conv4_3.bin", std::ios::binary); 
  }
  else if (10 == Lyr)
    feature.open("./data/conv5_1fp16.bin", std::ios::binary); 
  else if (11 == Lyr)
    feature.open("./data/conv5_2fp16.bin", std::ios::binary); 
  else if (12 == Lyr){
    if (Pooling)
      feature.open("./data/pool5fp16.bin", std::ios::binary); 
    else 
      feature.open("./data/conv5_3fp16.bin", std::ios::binary); 
  }

  int r_size = 0;
  if (Pooling)
    r_size = CHNEL[Lyr] * SHAPE[Lyr] * SHAPE[Lyr] >> 2;
  else
    r_size = CHNEL[Lyr] * SHAPE[Lyr] * SHAPE[Lyr];
  Dtype *ref_feat = (Dtype *) malloc(r_size * sizeof(Dtype));
  char *ref_char = reinterpret_cast<char *>(ref_feat); 
  feature.read(ref_char, r_size * sizeof(Dtype));

  int row_num = Pooling ? SHAPE[Lyr] / 2 : SHAPE[Lyr];
  int col_num = Pooling ? SHAPE[Lyr] / 2 : SHAPE[Lyr];
  for (int row = 0; row < row_num; row++) {
    for (int och = 0; och < CHNEL[Lyr]; och++) {
      for (int col = 0; col < col_num; col++) {
        int pos = row * CHNEL[Lyr] * row_num + och * col_num + col;
        Dtype ref = *(ref_feat + pos);
        Dtype out = *(Out + pos);
        if (ref < 10.0){
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
  feature.close();
  return;
}
