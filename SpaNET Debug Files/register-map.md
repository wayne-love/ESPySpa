# SpaNET SpaLINK Register Field Map

Fields are addressed as `RN+offset` where N is the register name and offset is the 1-based position within the CSV response line for that register.

**Write command notation:** `W##` = write command (prefixed with value), `S##` = state command (prefixed with value). Toggle commands (no value) are noted explicitly.

---

## R2 — Electrical / System Status

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R2+1 | `MainsCurrent` | Mains current draw ×10 (A). e.g. 77 = 7.7 A | — |
| R2+2 | `MainsVoltage` | Mains supply voltage (V) | — |
| R2+3 | `CaseTemperature` | Controller case temperature ×10 (°C) | — |
| R2+4 | `PortCurrent` | Port current draw ×10 (A) | — |
| R2+5 | `SpaDayOfWeek` | Day of week on spa RTC. 0=Mon…6=Sun | `S06:<0-6>` |
| R2+6 | *(SpaTime — hour)* | Hour component of spa RTC | `S04:<h>` |
| R2+7 | *(SpaTime — minute)* | Minute component of spa RTC | `S05:<m>` |
| R2+8 | *(SpaTime — second)* | Second component of spa RTC | — |
| R2+9 | *(SpaTime — day)* | Day-of-month component of spa RTC | `S03:<d>` |
| R2+10 | *(SpaTime — month)* | Month component of spa RTC | `S02:<m>` |
| R2+11 | *(SpaTime — year)* | Year component of spa RTC (2-digit) | `S01:<yy>` |
| R2+12 | `HeaterTemperature` | Heating element temperature ×10 (°C) | — |
| R2+13 | `PoolTemperature` | Pool/secondary sensor temperature ×10 (°C). Often unreliable (999.9); use `WTMP` instead | — |
| R2+14 | `WaterPresent` | Water presence detected. 1=Yes, 0=No | — |
| R2+15 | *(TempOffset)* | Temperature offset calibration. Not mapped in code | — |
| R2+16 | `AwakeMinutesRemaining` | Minutes before spa returns to sleep/standby | — |
| R2+17 | `FiltPumpRunTimeTotal` | Total cumulative filtration pump run time (minutes) | — |
| R2+18 | `FiltPumpReqMins` | Remaining filtration minutes required this cycle | — |
| R2+19 | `LoadTimeOut` | Load management timeout counter (seconds) | — |
| R2+20 | `HourMeter` | Total controller run time ×10 (hours). e.g. 279654 = 27965.4 h | — |
| R2+21 | `Relay1` | Relay 1 cumulative run time or state | — |
| R2+22 | `Relay2` | Relay 2 cumulative run time or state | — |
| R2+23 | `Relay3` | Relay 3 cumulative run time or state | — |
| R2+24 | `Relay4` | Relay 4 cumulative run time or state | — |
| R2+25 | `Relay5` | Relay 5 cumulative run time or state | — |
| R2+26 | `Relay6` | Relay 6 cumulative run time or state | — |
| R2+27 | `Relay7` | Relay 7 cumulative run time or state | — |
| R2+28 | `Relay8` | Relay 8 cumulative run time or state | — |
| R2+29 | `Relay9` | Relay 9 cumulative run time or state | — |

---

## R3 — Configuration / Identity

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R3+1 | `CLMT` | Current limit (A). Max load allowed on supply. Range 10–60 (OEM C.LMT setting) | — |
| R3+2 | `PHSE` | Mains phase configuration. 1=Single, 2=Dual, 3=Three phase | — |
| R3+3 | `LLM1` | Phase 1 load limit — max simultaneous loads on phase 1. Range 1–5 (OEM x.LLM) | — |
| R3+4 | `LLM2` | Phase 2 load limit. Range 1–5 | — |
| R3+5 | `LLM3` | Phase 3 load limit. Range 1–5 | — |
| R3+6 | `SVER` | Firmware version string (e.g. `SW V6 19 11 12`) | — |
| R3+7 | `Model` | Controller model string (e.g. `SV3`) | — |
| R3+8 | `SerialNo1` | Serial number part 1 | — |
| R3+9 | `SerialNo2` | Serial number part 2 | — |
| R3+10 | `D1` | Dipswitch 1: Circulation pump fitted (1=Fitted) | — |
| R3+11 | `D2` | Dipswitch 2: Pump 1 type (1=Two Speed; if 0, Pump 2 assumed fitted) | — |
| R3+12 | `D3` | Dipswitch 3: SV3 = Pump 3 fitted; SV2/SV4 = Pump 3 two-speed | — |
| R3+13 | `D4` | Dipswitch 4: SV2/SV4 = Pump 4 fitted; SV3 = Not used | — |
| R3+14 | `D5` | Dipswitch 5: Phase input (1=2/3 Phase, 0=Single). Enables D6 when set | — |
| R3+15 | `D6` | Dipswitch 6: Multi-phase type when D5=1 (1=Three Phase, 0=Two Phase) | — |
| R3+16 | `Pump` | Pump configuration string | — |
| R3+17 | `LS` | Load shedding level | — |
| R3+18 | `HV` | High voltage input active (bool) | — |
| R3+19 | `SnpMR` | Snooze mode remaining time (MR). Per-second countdown; 0 when not in snooze | — |
| R3+20 | `Status` | Operational status string (e.g. `Filtering`, `In use`, `Heating`) | — |
| R3+21 | `PrimeCount` | Priming cycle count | — |
| R3+22 | `EC` | Variable heat element current draw x10 (A) | — |
| R3+23 | `HAMB` | Heater ambient air temperature ×10 (°C) | — |
| R3+24 | `HCON` | Heater conductivity/connection value | — |
| R3+25 | *(HV_2)* | High voltage 2 input state. Encoding unconfirmed; not mapped in code | — |

---

## R4 — Power / Timers / Mode

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R4+1 | `Mode` | Spa operating mode. 0=NORM, 1=ECON, 2=AWAY, 3=WEEK | `W66:<0-3>` |
| R4+2 | `Ser1_Timer` | Service interval 1 countdown (hours remaining) | — |
| R4+3 | `Ser2_Timer` | Service interval 2 countdown (hours remaining) | — |
| R4+4 | `Ser3_Timer` | Service interval 3 countdown (hours remaining) | — |
| R4+5 | `HeatMode` | Heating source mode. 0=Element, 1=Heat pump, 2=Both, 3=Cool | — |
| R4+6 | `PumpIdleTimer` | Minutes since last pump activity | — |
| R4+7 | `PumpRunTimer` | Continuous pump run time (seconds, increments each second) | — |
| R4+8 | `AdtPoolHys` | Adaptive pool temperature hysteresis ×10 (°C) | — |
| R4+9 | `AdtHeaterHys` | Adaptive heater temperature hysteresis ×10 (°C) | — |
| R4+10 | `Power` | Instantaneous power consumption ×10 (W). e.g. 35 = 3.5 kW | — |
| R4+11 | `Power_kWh` | Cumulative energy consumption ×100 (kWh) | — |
| R4+12 | `Power_Today` | Energy consumed today ×10 (Wh) | — |
| R4+13 | `Power_Yesterday` | Energy consumed yesterday ×10 (Wh) | — |
| R4+14 | `ThermalCutOut` | Thermal cut-out trip count | — |
| R4+15 | `Test_D1` | Diagnostic output D1 state | — |
| R4+16 | `Test_D2` | Diagnostic output D2 state | — |
| R4+17 | `Test_D3` | Diagnostic output D3 state | — |
| R4+18 | `ElementHeatSourceOffset` | Element heat source temperature offset ×10 (°C) | — |
| R4+19 | `Frequency` | Detected mains frequency (Hz) | — |
| R4+20 | `HPHeatSourceOffset_Heat` | Heat pump heat-mode source temperature offset ×10 (°C) | — |
| R4+21 | `HPHeatSourceOffset_Cool` | Heat pump cool-mode source temperature offset ×10 (°C) | — |
| R4+22 | `HeatSourceOffTime` | Heat source off-time (minutes) | — |
| R4+23 | `Vari_Mode` | Variable speed pump mode. 0=Auto, 1=Manual | — |
| R4+24 | `Vari_Speed` | Variable speed pump current speed setting | — |
| R4+25 | `Vari_Percent` | Variable speed pump output percentage (%) | — |

---

## R5 — Touchpad / Pump / Water States

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R5+1 | *(TouchPad2)* | Touchpad 2 type code. Encoding unknown; not mapped in code | — |
| R5+2 | *(TouchPad1)* | Touchpad 1 type code. Encoding unknown; not mapped in code | — |
| R5+3–4 | *(unknown)* | Unmapped. Possibly pump button states in touchpad encoding | — |
| R5+5 | *(RB_TP_Blower)* | Blower/air injector state. Encoding unknown; not populated in code | — |
| R5+6–9 | *(unknown)* | Unmapped fields | — |
| R5+10 | `RB_TP_Sleep` | True when spa is sleeping due to a sleep timer | — |
| R5+11 | `RB_TP_Ozone` | True when ozone/UV sanitiser is running | — |
| R5+12 | `RB_TP_Heater` | True when heating or cooling is actively running | — |
| R5+13 | `RB_TP_Auto` | True when auto mode is active | — |
| R5+14 | `RB_TP_Light` | Light state. 0=Off, 1=On | `W14` (toggle — no value argument) |
| R5+15 | `WTMP` | Actual (measured) water temperature ×10 (°C). e.g. 376 = 37.6°C | — |
| R5+16 | `CleanCycle` | True when a sanitise/clean cycle is in progress | — |
| R5+17 | *(unknown)* | Per-second countdown timer. Behaviour consistent with a touchpad activity timeout (~30 s); resets on keypad interaction | — |
| R5+18 | `RB_TP_Pump1` | Pump 1 operating state. 0=Off, 1=Speed 1, 2=Speed 2, 4=Auto | `S22:<0-4>` |
| R5+19 | `RB_TP_Pump2` | Pump 2 operating state. 0=Off, 1=On, 4=Auto | `S23:<0-4>` |
| R5+20 | `RB_TP_Pump3` | Pump 3 operating state. 0=Off, 1=On, 4=Auto | `S24:<0-4>` |
| R5+21 | `RB_TP_Pump4` | Pump 4 operating state. 0=Off, 1=On, 4=Auto | `S25:<0-4>` |
| R5+22 | `RB_TP_Pump5` | Pump 5 operating state. 0=Off, 1=On, 4=Auto | `S26:<0-4>` |

---

## R6 — Settings / Lighting / Scheduling

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R6+1 | `VARIValue` | Variable pump/blower speed level. Range 1–5 | `S13:<1-5>` |
| R6+2 | `LBRTValue` | Light brightness level. Range 1–5 | `S08:<1-5>` |
| R6+3 | `CurrClr` | Light colour index (hue angle mapped to controller colour value). Range 0–31 | `S10:<0-31>` |
| R6+4 | `ColorMode` | Light effect mode. 0=White, 1=Color, 2=Fade, 3=Step, 4=Party | `S07:<0-4>` |
| R6+5 | `LSPDValue` | Light effect speed. Range 1–5 | `S09:<1-5>` |
| R6+6 | `FiltHrs` | Filtration run time per block (hours). Range 1–24 | `W60:<1-24>` |
| R6+7 | `FiltBlockHrs` | Filtration cycle block duration (hours). Valid: 1,2,3,4,6,8,12,24 | `W90:<hrs>` |
| R6+8 | `STMP` | Water temperature set point ×10 (°C). Range 50–410 (5.0–41.0°C) | `W40:<value>` |
| R6+9 | `L_24HOURS` | 24-hour continuous operation flag. 0=Off, 1=On | — |
| R6+10 | `PSAV_LVL` | Power save level. 0=Off, 1=Low, 2=High | — |
| R6+11 | `PSAV_BGN` | Power save start time encoded as h×256+m | — |
| R6+12 | `PSAV_END` | Power save end time encoded as h×256+m | — |
| R6+13 | `L_1SNZ_DAY` | Sleep timer 1 day bitmap. 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays | `W67:<value>` |
| R6+14 | `L_2SNZ_DAY` | Sleep timer 2 day bitmap. Same encoding as L_1SNZ_DAY | `W70:<value>` |
| R6+15 | `L_1SNZ_BGN` | Sleep timer 1 start time encoded as h×256+m | `W68:<value>` |
| R6+16 | `L_2SNZ_BGN` | Sleep timer 2 start time encoded as h×256+m | `W71:<value>` |
| R6+17 | `L_1SNZ_END` | Sleep timer 1 finish time encoded as h×256+m | `W69:<value>` |
| R6+18 | `L_2SNZ_END` | Sleep timer 2 finish time encoded as h×256+m | `W72:<value>` |
| R6+19 | `DefaultScrn` | Default touchpad screen index shown on wake | — |
| R6+20 | `TOUT` | Pump and blower auto time-out duration (minutes). Range 10–60 | — |
| R6+21 | `VPMP` | Variable-speed pump fitted (bool) | — |
| R6+22 | `HIFI` | HiFi audio output enabled (bool) | — |
| R6+23 | `BRND` | OEM brand identifier | — |
| R6+24 | `PRME` | Priming mode setting *(V3 firmware only)* | — |
| R6+25 | `ELMT` | Heating element type/configuration *(V3 firmware only)* | — |
| R6+26 | `TYPE` | System type identifier *(V3 firmware only)* | — |
| R6+27 | `GAS` | Gas heating installed. 0=No, 1=Yes *(V3 firmware only)* | — |

---

## R7 — Calibration / Sanitise / Advanced Settings

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| R7+1 | `WCLNTime` | Auto sanitise cycle start time encoded as h×256+m. e.g. 3072 = 12:00 | — |
| R7+2 | `OzoneOff` | Ozone/sanitiser manually disabled (bool). *Note: R7+2 and R7+3 may be swapped in some firmware* | — |
| R7+3 | `TemperatureUnits` | Temperature display units. 0=°C, 1=°F. *Note: may be swapped with R7+2* | — |
| R7+4 | `Ozone24` | Ozone/sanitiser continuous 24h mode (bool) | — |
| R7+5 | `CJET` | Circulation jet boost active (bool) | — |
| R7+6 | `Circ24` | Circulation pump continuous 24h mode (bool) | — |
| R7+7 | `VELE` | Variable-power element operation enabled (bool) | — |
| R7+8 | *(StartDD)* | Install date — day. Not mapped in code | — |
| R7+9 | *(StartMM)* | Install date — month. Not mapped in code | — |
| R7+10 | *(StartYY)* | Install date — year. Not mapped in code | — |
| R7+11 | `V_Max` | Maximum mains voltage recorded this session (V) | — |
| R7+12 | `V_Min` | Minimum mains voltage recorded this session (V) | — |
| R7+13 | `V_Max_24` | Maximum mains voltage in last 24 hours (V) | — |
| R7+14 | `V_Min_24` | Minimum mains voltage in last 24 hours (V) | — |
| R7+15 | `CurrentZero` | Current sensor zero-point calibration offset | — |
| R7+16 | `CurrentAdjust` | Current sensor gain calibration ×10 | — |
| R7+17 | `VoltageAdjust` | Voltage sensor calibration ×10 | — |
| R7+18 | *(unknown)* | Unlabelled field. Observed value 3 | — |
| R7+19 | `Ser1` | Service interval 1 period (hours) | — |
| R7+20 | `Ser2` | Service interval 2 period (hours) | — |
| R7+21 | `Ser3` | Service interval 3 period (hours) | — |
| R7+22 | `VMAX` | Maximum permitted supply voltage (V) | — |
| R7+23 | `AHYS` | Adaptive hysteresis setting ×10 (°C) | — |
| R7+24 | `HUSE` | Heat pump suspended during spa use when false (H.USE OEM setting) | — |
| R7+25 | `HELE` | Auxiliary booster element. false=Off, true=On | `W98:<0-1>` |
| R7+26 | `HPMP` | Heat pump operating mode. 0=Auto, 1=Heat, 2=Cool, 3=Off | `W99:<0-3>` |
| R7+27 | `PMIN` | Minimum power level for load management ×10 (kW) | — |
| R7+28 | `PFLT` | Filtration pump power draw ×10 (kW) | — |
| R7+29 | `PHTR` | Heater element power draw ×10 (kW) | — |
| R7+30 | `PMAX` | Maximum total power for load management ×10 (kW) | — |

---

## R9 / RA / RB — Fault Logs (F1=most recent, F2, F3)

Each fault register follows the same layout. R9=F1, RA=F2, RB=F3.

| Offset | Property (F1 / F2 / F3) | Description |
|--------|-------------------------|-------------|
| +1 | *(fault label)* | Fault identifier string (`F1`, `F2`, `F3`) |
| +2 | `F1_HR` / `F2_HR` / `F3_HR` | Hour meter reading at time of fault ×10 (hours) |
| +3 | `F1_Time` / `F2_Time` / `F3_Time` | Time of fault encoded as h×256+m |
| +4 | `F1_ER` / `F2_ER` / `F3_ER` | Error/fault code |
| +5 | `F1_I` / `F2_I` / `F3_I` | Current reading ×10 (A) at time of fault |
| +6 | `F1_V` / `F2_V` / `F3_V` | Voltage reading (V) at time of fault |
| +7 | `F1_PT` / `F2_PT` / `F3_PT` | Pool temperature ×10 (°C) at time of fault |
| +8 | `F1_HT` / `F2_HT` / `F3_HT` | Heater temperature ×10 (°C) at time of fault |
| +9 | `F1_CT` / `F2_CT` / `F3_CT` | Case temperature ×10 (°C) at time of fault |
| +10 | `F1_PU` / `F2_PU` / `F3_PU` | Pump state at time of fault |
| +11 | `F1_VE` / `F2_VE` / `F3_VE` | Variable element state at time of fault (bool) |
| +12 | `F1_ST` / `F2_ST` / `F3_ST` | System status code at time of fault |

---

## RC — Relay / Outlet States

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| RC+1 | *(Outlet_Heater)* | Heating element relay output state. Encoding not confirmed; not mapped in code | — |
| RC+2 | *(Outlet_Circ)* | Circulation pump relay output state. Not mapped in code | — |
| RC+3 | *(Outlet_Sanitise)* | Sanitiser relay output state. Not mapped in code | — |
| RC+4 | *(Outlet_Pump1)* | Pump 1 relay output state. Not mapped in code | — |
| RC+5 | *(Outlet_Pump2)* | Pump 2 relay output state. Not mapped in code | — |
| RC+6–9 | *(unknown)* | Unmapped relay fields | — |
| RC+10 | `Outlet_Blower` | Air blower mode. 0=Variable, 1=Ramp, 2=Off | `S28:<0-2>` |

---

## RE — Heat Pump

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| RE+1 | `HP_Present` | Heat pump unit installed. 0=No, 1=Yes | — |
| RE+2–9 | *(HP switches)* | HP flow switch, high/low pressure switches, cut-outs, D1–D3 states. Not mapped in code | — |
| RE+10 | `HP_Ambient` | Heat pump ambient air temperature ×10 (°C) | — |
| RE+11 | `HP_Condensor` | Heat pump condenser temperature ×10 (°C) | — |
| RE+12 | `HP_Compressor_State` | Compressor running (bool) | — |
| RE+13 | `HP_Fan_State` | Fan running (bool) | — |
| RE+14 | `HP_4W_Valve` | 4-way reversing valve active — cool mode (bool) | — |
| RE+15 | `HP_Heater_State` | Auxiliary heater element active (bool) | — |
| RE+16 | `HP_State` | Heat pump operating state code | — |
| RE+17 | `HP_Mode` | Heat pump mode. 0=Auto, 1=Heat, 2=Cool, 3=Off | — |
| RE+18 | `HP_Defrost_Timer` | Defrost cycle timer (minutes) | — |
| RE+19 | `HP_Comp_Run_Timer` | Compressor cumulative run time (minutes) | — |
| RE+20 | `HP_Low_Temp_Timer` | Low ambient temperature protection timer (minutes) | — |
| RE+21 | `HP_Heat_Accum_Timer` | Heat accumulation timer (minutes) | — |
| RE+22 | `HP_Sequence_Timer` | Start sequence timer (seconds) | — |
| RE+23 | `HP_Warning` | Warning/fault code. 0 = no warning | — |
| RE+24 | `FrezTmr` | Freeze protection activation timer (minutes) | — |
| RE+25 | `DBGN` | Defrost cycle start temperature ×10 (°C) | — |
| RE+26 | `DEND` | Defrost cycle end temperature ×10 (°C) | — |
| RE+27 | `DCMP` | Defrost compressor run time limit (minutes) | — |
| RE+28 | `DMAX` | Defrost maximum duration (minutes) | — |
| RE+29 | `DELE` | Defrost element activation delay (minutes) | — |
| RE+30 | `DPMP` | Defrost pump operating mode | — |

---

## RG — Pump Installation / Lock (V3 firmware only)

| Offset | Property | Description | Write Command |
|--------|----------|-------------|---------------|
| RG+1 | `Pump1OkToRun` | Pump 1 safe to start (bool) | — |
| RG+2 | `Pump2OkToRun` | Pump 2 safe to start (bool) | — |
| RG+3 | `Pump3OkToRun` | Pump 3 safe to start (bool) | — |
| RG+4 | `Pump4OkToRun` | Pump 4 safe to start (bool) | — |
| RG+5 | `Pump5OkToRun` | Pump 5 safe to start (bool) | — |
| RG+6 | *(unknown)* | Unmapped | — |
| RG+7 | `Pump1InstallState` | Pump 1 install config string. Format: `I-S-PPP` (I=installed, S=speed type, PPP=valid states e.g. `014`=Off/On/Auto) | — |
| RG+8 | `Pump2InstallState` | Pump 2 install config string | — |
| RG+9 | `Pump3InstallState` | Pump 3 install config string | — |
| RG+10 | `Pump4InstallState` | Pump 4 install config string | — |
| RG+11 | `Pump5InstallState` | Pump 5 install config string | — |
| RG+12 | `LockMode` | Keypad lock mode. 0=Unlocked, 1=Partially Locked, 2=Locked | `S21:<0-2>` |
| RG+13–14 | *(unknown)* | Unmapped. Observed as 0 | — |
| RG+15 | *(unknown)* | Unmapped. Observed value ~3727 | — |

---

## Write Command Reference

| Command | Property | Notes |
|---------|----------|-------|
| `S01:<yy>` | SpaTime — year | 2-digit year |
| `S02:<m>` | SpaTime — month | |
| `S03:<d>` | SpaTime — day | |
| `S04:<h>` | SpaTime — hour | |
| `S05:<m>` | SpaTime — minute | |
| `S06:<0-6>` | SpaDayOfWeek | 0=Mon…6=Sun |
| `S07:<0-4>` | ColorMode | 0=White, 1=Color, 2=Fade, 3=Step, 4=Party |
| `S08:<1-5>` | LBRTValue | Light brightness |
| `S09:<1-5>` | LSPDValue | Light effect speed |
| `S10:<0-31>` | CurrClr | Light colour index |
| `S13:<1-5>` | VARIValue | Variable pump/blower speed |
| `S21:<0-2>` | LockMode | 0=Unlocked, 1=Partial, 2=Locked |
| `S22:<0-4>` | RB_TP_Pump1 | 0=Off, 1=Spd1, 2=Spd2, 4=Auto |
| `S23:<0-4>` | RB_TP_Pump2 | |
| `S24:<0-4>` | RB_TP_Pump3 | |
| `S25:<0-4>` | RB_TP_Pump4 | |
| `S26:<0-4>` | RB_TP_Pump5 | |
| `S28:<0-2>` | Outlet_Blower | 0=Variable, 1=Ramp, 2=Off |
| `W14` | RB_TP_Light | **Toggle** — no value argument |
| `W40:<value>` | STMP | Set point ×10. e.g. `W40:380` = 38.0°C |
| `W60:<1-24>` | FiltHrs | Filtration hours per block |
| `W66:<0-3>` | Mode | 0=NORM, 1=ECON, 2=AWAY, 3=WEEK |
| `W67:<value>` | L_1SNZ_DAY | Day bitmap: 128=Off, 127=Everyday, 96=Weekends, 31=Weekdays |
| `W68:<value>` | L_1SNZ_BGN | Sleep timer 1 start: h×256+m |
| `W69:<value>` | L_1SNZ_END | Sleep timer 1 end: h×256+m |
| `W70:<value>` | L_2SNZ_DAY | Day bitmap (same as W67) |
| `W71:<value>` | L_2SNZ_BGN | Sleep timer 2 start: h×256+m |
| `W72:<value>` | L_2SNZ_END | Sleep timer 2 end: h×256+m |
| `W90:<hrs>` | FiltBlockHrs | Valid: 1,2,3,4,6,8,12,24 |
| `W98:<0-1>` | HELE | Aux booster element |
| `W99:<0-3>` | HPMP | 0=Auto, 1=Heat, 2=Cool, 3=Off |
