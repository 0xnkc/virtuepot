PROGRAM water_tank
VAR
  level: REAL;
  areal: REAL;
  height: REAL;
  inflow: REAL;
  inflow_mean: REAL;
  inflow_stddev: REAL;
  max_inflow: REAL;
  outflow: REAL;
  max_outflow: REAL;
  set_level: REAL;
END_VAR


  level := 1000.0;
  areal := 1000000.0;
  height := 2000.0;
  inflow := 20.0;
  inflow_mean := 20.0;
  inflow_stddev := 3.0;
  max_inflow := 40.0;
  outflow := 20.0;
  max_outflow := 40.0;
  set_level := 1000.0;

  WHILE (TRUE) DO
    inflow := inflow_mean + (RANDOM - 0.5) * inflow_stddev;
    IF (inflow > max_inflow) THEN
      inflow := max_inflow;
    END_IF;

    level := level + inflow - outflow;

    IF (level > height) THEN
      level := height;
    END_IF;

    IF (level < 0) THEN
      level := 0;
    END_IF;

    outflow := (level - set_level) * areal;

    IF (outflow > max_outflow) THEN
      outflow := max_outflow;
    END_IF;
  END_WHILE;
END_PROGRAM
