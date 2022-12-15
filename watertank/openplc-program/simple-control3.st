PROGRAM Hello_World
  VAR
    level AT %IW101 : UINT;
    outflow AT %QW100 : UINT;
    max_level AT %QW1 : UINT := 1200;
    min_level AT %QW2 : UINT := 800;
  END_VAR
  VAR
    convert_to_level : REAL := 32.768;
    convert_from_flow : REAL := 1638.4;
    low_flow : REAL := 15.0;
    high_flow : REAL := 25.0;
    UINT_TO_REAL3_OUT : REAL;
    DIV10_OUT : REAL;
    UINT_TO_REAL6_OUT : REAL;
    GT11_OUT : BOOL;
    UINT_TO_REAL9_OUT : REAL;
    LT13_OUT : BOOL;
    SEL16_OUT : REAL;
    SEL15_OUT : REAL;
    MUL8_OUT : REAL;
    REAL_TO_UINT4_OUT : UINT;
  END_VAR

  UINT_TO_REAL3_OUT := UINT_TO_REAL(level);
  DIV10_OUT := DIV(UINT_TO_REAL3_OUT, convert_to_level);
  UINT_TO_REAL6_OUT := UINT_TO_REAL(max_level);
  GT11_OUT := GT(DIV10_OUT, UINT_TO_REAL6_OUT);
  UINT_TO_REAL9_OUT := UINT_TO_REAL(min_level);
  LT13_OUT := LT(DIV10_OUT, UINT_TO_REAL9_OUT);
  SEL16_OUT := SEL(LT13_OUT, SEL15_OUT, low_flow);
  SEL15_OUT := SEL(GT11_OUT, SEL16_OUT, high_flow);
  MUL8_OUT := MUL(SEL15_OUT, convert_from_flow);
  REAL_TO_UINT4_OUT := REAL_TO_UINT(MUL8_OUT);
  outflow := REAL_TO_UINT4_OUT;
END_PROGRAM


CONFIGURATION Config0

  RESOURCE Res0 ON PLC
    TASK task0(INTERVAL := T#300ms,PRIORITY := 0);
    PROGRAM instance0 WITH task0 : Hello_World;
  END_RESOURCE
END_CONFIGURATION
