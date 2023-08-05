PROGRAM Main
  VAR
    I_PbFill AT %IX100.0 : BOOL;
    I_PbDischarge AT %IX100.1 : BOOL := True;
    Q_FillValve AT %QX100.0 : BOOL;
    Q_FillLight AT %QX100.1 : BOOL;
    Q_DischargeValve AT %QX100.2 : BOOL;
    Q_LightDischarge AT %QX100.3 : BOOL;
    Q_Display AT %QW100 : INT;
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
    Simulation AT %QW101 : INT;
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
