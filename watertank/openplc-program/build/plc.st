TYPE
  LOGLEVEL : (CRITICAL, WARNING, INFO, DEBUG) := INFO;
END_TYPE

FUNCTION_BLOCK LOGGER
  VAR_INPUT
    TRIG : BOOL;
    MSG : STRING;
    LEVEL : LOGLEVEL := INFO;
  END_VAR
  VAR
    TRIG0 : BOOL;
  END_VAR

  IF TRIG AND NOT TRIG0 THEN
  {{
   LogMessage(GetFbVar(LEVEL),(char*)GetFbVar(MSG, .body),GetFbVar(MSG, .len));
  }}
  END_IF;
  TRIG0:=TRIG;
END_FUNCTION_BLOCK


PROGRAM Main
  VAR
    I_PbFill : BOOL;
    I_PbDischarge : BOOL := True;
    Q_FillValve : BOOL;
    Q_FillLight : BOOL;
    Q_DischargeValve : BOOL;
    Q_LightDischarge : BOOL;
    Q_Display : INT;
  END_VAR
  VAR
    Filling : BOOL;
    Discharging : BOOL;
    TimeFilling : TIME;
    TimeFillingInt : INT;
    TimeDischarging : TIME;
    TimeDischargingInt : INT;
    TON0 : TON;
    RS0 : RS;
    R_TRIG0 : R_TRIG;
    TON1 : TON;
    RS1 : RS;
    F_TRIG0 : F_TRIG;
    Placeholder : INT;
  END_VAR
  VAR
    Simulation : INT;
  END_VAR
  VAR
    _TMP_NOT11_OUT : BOOL;
    _TMP_AND12_OUT : BOOL;
    _TMP_NOT21_OUT : BOOL;
    _TMP_AND22_OUT : BOOL;
    _TMP_TIME_TO_INT27_OUT : INT;
    _TMP_TIME_TO_INT28_OUT : INT;
    _TMP_ADD57_OUT : INT;
  END_VAR

  R_TRIG0(CLK := I_PbFill);
  _TMP_NOT11_OUT := NOT(Discharging);
  _TMP_AND12_OUT := AND(R_TRIG0.Q, _TMP_NOT11_OUT);
  TON0(IN := Filling, PT := T#8s);
  RS0(S := _TMP_AND12_OUT, R1 := TON0.Q);
  Filling := RS0.Q1;
  TimeFilling := TON0.ET;
  F_TRIG0(CLK := I_PbDischarge);
  _TMP_NOT21_OUT := NOT(Filling);
  _TMP_AND22_OUT := AND(F_TRIG0.Q, _TMP_NOT21_OUT);
  TON1(IN := Discharging, PT := T#8s);
  RS1(S := _TMP_AND22_OUT, R1 := TON1.Q);
  Discharging := RS1.Q1;
  TimeDischarging := TON1.ET;
  _TMP_TIME_TO_INT27_OUT := TIME_TO_INT(TimeFilling);
  TimeFillingInt := _TMP_TIME_TO_INT27_OUT;
  _TMP_TIME_TO_INT28_OUT := TIME_TO_INT(TimeDischarging);
  TimeDischargingInt := _TMP_TIME_TO_INT28_OUT;
  Q_FillLight := Filling;
  Q_FillValve := Filling;
  Q_LightDischarge := Discharging;
  Q_DischargeValve := Discharging;
  _TMP_ADD57_OUT := ADD(TimeFillingInt, TimeDischargingInt);
  Q_Display := _TMP_ADD57_OUT;
END_PROGRAM


CONFIGURATION Config0

  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#20ms,PRIORITY := 0);
    PROGRAM instance0 WITH task0 : Main;
  END_RESOURCE
END_CONFIGURATION
