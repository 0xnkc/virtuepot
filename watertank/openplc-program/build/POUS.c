#include "POUS.h"
void LOGGER_init__(LOGGER *data__, BOOL retain) {
  __INIT_VAR(data__->EN,__BOOL_LITERAL(TRUE),retain)
  __INIT_VAR(data__->ENO,__BOOL_LITERAL(TRUE),retain)
  __INIT_VAR(data__->TRIG,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->MSG,__STRING_LITERAL(0,""),retain)
  __INIT_VAR(data__->LEVEL,LOGLEVEL__INFO,retain)
  __INIT_VAR(data__->TRIG0,__BOOL_LITERAL(FALSE),retain)
}

// Code part
void LOGGER_body__(LOGGER *data__) {
  // Control execution
  if (!__GET_VAR(data__->EN)) {
    __SET_VAR(data__->,ENO,,__BOOL_LITERAL(FALSE));
    return;
  }
  else {
    __SET_VAR(data__->,ENO,,__BOOL_LITERAL(TRUE));
  }
  // Initialise TEMP variables

  if ((__GET_VAR(data__->TRIG,) && !(__GET_VAR(data__->TRIG0,)))) {
    #define GetFbVar(var,...) __GET_VAR(data__->var,__VA_ARGS__)
    #define SetFbVar(var,val,...) __SET_VAR(data__->,var,__VA_ARGS__,val)

   LogMessage(GetFbVar(LEVEL),(char*)GetFbVar(MSG, .body),GetFbVar(MSG, .len));
  
    #undef GetFbVar
    #undef SetFbVar
;
  };
  __SET_VAR(data__->,TRIG0,,__GET_VAR(data__->TRIG,));

  goto __end;

__end:
  return;
} // LOGGER_body__() 





void MAIN_init__(MAIN *data__, BOOL retain) {
  __INIT_VAR(data__->I_PBFILL,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->I_PBDISCHARGE,__BOOL_LITERAL(TRUE),retain)
  __INIT_VAR(data__->Q_FILLVALVE,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->Q_FILLLIGHT,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->Q_DISCHARGEVALVE,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->Q_LIGHTDISCHARGE,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->Q_DISPLAY,0,retain)
  __INIT_VAR(data__->FILLING,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->DISCHARGING,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->TIMEFILLING,__time_to_timespec(1, 0, 0, 0, 0, 0),retain)
  __INIT_VAR(data__->TIMEFILLINGINT,0,retain)
  __INIT_VAR(data__->TIMEDISCHARGING,__time_to_timespec(1, 0, 0, 0, 0, 0),retain)
  __INIT_VAR(data__->TIMEDISCHARGINGINT,0,retain)
  TON_init__(&data__->TON0,retain);
  RS_init__(&data__->RS0,retain);
  R_TRIG_init__(&data__->R_TRIG0,retain);
  TON_init__(&data__->TON1,retain);
  RS_init__(&data__->RS1,retain);
  F_TRIG_init__(&data__->F_TRIG0,retain);
  __INIT_VAR(data__->PLACEHOLDER,0,retain)
  __INIT_VAR(data__->SIMULATION,0,retain)
  __INIT_VAR(data__->_TMP_NOT11_OUT,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->_TMP_AND12_OUT,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->_TMP_NOT21_OUT,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->_TMP_AND22_OUT,__BOOL_LITERAL(FALSE),retain)
  __INIT_VAR(data__->_TMP_TIME_TO_INT27_OUT,0,retain)
  __INIT_VAR(data__->_TMP_TIME_TO_INT28_OUT,0,retain)
  __INIT_VAR(data__->_TMP_ADD57_OUT,0,retain)
}

// Code part
void MAIN_body__(MAIN *data__) {
  // Initialise TEMP variables

  __SET_VAR(data__->R_TRIG0.,CLK,,__GET_VAR(data__->I_PBFILL,));
  R_TRIG_body__(&data__->R_TRIG0);
  __SET_VAR(data__->,_TMP_NOT11_OUT,,!(__GET_VAR(data__->DISCHARGING,)));
  __SET_VAR(data__->,_TMP_AND12_OUT,,AND__BOOL__BOOL(
    (BOOL)__BOOL_LITERAL(TRUE),
    NULL,
    (UINT)2,
    (BOOL)__GET_VAR(data__->R_TRIG0.Q,),
    (BOOL)__GET_VAR(data__->_TMP_NOT11_OUT,)));
  __SET_VAR(data__->TON0.,IN,,__GET_VAR(data__->FILLING,));
  __SET_VAR(data__->TON0.,PT,,__time_to_timespec(1, 0, 8, 0, 0, 0));
  TON_body__(&data__->TON0);
  __SET_VAR(data__->RS0.,S,,__GET_VAR(data__->_TMP_AND12_OUT,));
  __SET_VAR(data__->RS0.,R1,,__GET_VAR(data__->TON0.Q,));
  RS_body__(&data__->RS0);
  __SET_VAR(data__->,FILLING,,__GET_VAR(data__->RS0.Q1,));
  __SET_VAR(data__->,TIMEFILLING,,__GET_VAR(data__->TON0.ET,));
  __SET_VAR(data__->F_TRIG0.,CLK,,__GET_VAR(data__->I_PBDISCHARGE,));
  F_TRIG_body__(&data__->F_TRIG0);
  __SET_VAR(data__->,_TMP_NOT21_OUT,,!(__GET_VAR(data__->FILLING,)));
  __SET_VAR(data__->,_TMP_AND22_OUT,,AND__BOOL__BOOL(
    (BOOL)__BOOL_LITERAL(TRUE),
    NULL,
    (UINT)2,
    (BOOL)__GET_VAR(data__->F_TRIG0.Q,),
    (BOOL)__GET_VAR(data__->_TMP_NOT21_OUT,)));
  __SET_VAR(data__->TON1.,IN,,__GET_VAR(data__->DISCHARGING,));
  __SET_VAR(data__->TON1.,PT,,__time_to_timespec(1, 0, 8, 0, 0, 0));
  TON_body__(&data__->TON1);
  __SET_VAR(data__->RS1.,S,,__GET_VAR(data__->_TMP_AND22_OUT,));
  __SET_VAR(data__->RS1.,R1,,__GET_VAR(data__->TON1.Q,));
  RS_body__(&data__->RS1);
  __SET_VAR(data__->,DISCHARGING,,__GET_VAR(data__->RS1.Q1,));
  __SET_VAR(data__->,TIMEDISCHARGING,,__GET_VAR(data__->TON1.ET,));
  __SET_VAR(data__->,_TMP_TIME_TO_INT27_OUT,,TIME_TO_INT(
    (BOOL)__BOOL_LITERAL(TRUE),
    NULL,
    (TIME)__GET_VAR(data__->TIMEFILLING,)));
  __SET_VAR(data__->,TIMEFILLINGINT,,__GET_VAR(data__->_TMP_TIME_TO_INT27_OUT,));
  __SET_VAR(data__->,_TMP_TIME_TO_INT28_OUT,,TIME_TO_INT(
    (BOOL)__BOOL_LITERAL(TRUE),
    NULL,
    (TIME)__GET_VAR(data__->TIMEDISCHARGING,)));
  __SET_VAR(data__->,TIMEDISCHARGINGINT,,__GET_VAR(data__->_TMP_TIME_TO_INT28_OUT,));
  __SET_VAR(data__->,Q_FILLLIGHT,,__GET_VAR(data__->FILLING,));
  __SET_VAR(data__->,Q_FILLVALVE,,__GET_VAR(data__->FILLING,));
  __SET_VAR(data__->,Q_LIGHTDISCHARGE,,__GET_VAR(data__->DISCHARGING,));
  __SET_VAR(data__->,Q_DISCHARGEVALVE,,__GET_VAR(data__->DISCHARGING,));
  __SET_VAR(data__->,_TMP_ADD57_OUT,,ADD__INT__INT(
    (BOOL)__BOOL_LITERAL(TRUE),
    NULL,
    (UINT)2,
    (INT)__GET_VAR(data__->TIMEFILLINGINT,),
    (INT)__GET_VAR(data__->TIMEDISCHARGINGINT,)));
  __SET_VAR(data__->,Q_DISPLAY,,__GET_VAR(data__->_TMP_ADD57_OUT,));

  goto __end;

__end:
  return;
} // MAIN_body__() 





