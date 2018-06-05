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
#include "../../include/utils/check.h"


bool dataflow_check(Dtype * Ref, Dtype * Res, int Cnt)
{
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
