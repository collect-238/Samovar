#ifndef _SAMOVAR_LUA_H_
#define _SAMOVAR_LUA_H_

#include <LuaWrapper.h>
LuaWrapper lua;

#include <SimpleMap.h>

unsigned long lua_timer[9]; //10 таймеров для lua
String lua_type_script;
String script1, script2, glv;

SimpleMap<String, String> *luaObj = new SimpleMap<String, String>([](String &a, String &b) -> int {
  if (a == b) return 0;      // a and b are equal
  else if (a > b) return 1;  // a is bigger than b
  else return -1;            // a is smaller than b
});

TaskHandle_t DoLuaScriptTask = NULL;
volatile bool lua_finished;
void IRAM_ATTR do_lua_script(void *parameter);

String get_lua_mode_name();
void load_lua_script();

void set_power(bool On);
void SendMsg(String m, MESSAGE_TYPE msg_type);
String get_global_variables();
void open_valve(bool Val);
void set_current_power(float Volt);
void set_body_temp();
void IRAM_ATTR set_mixer(bool On);
void set_alarm();
void IRAM_ATTR pause_withdrawal(bool Pause);

static int lua_wrapper_pinMode(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  int b = luaL_checkinteger(lua_state, 2);
  pinMode(a, b);
  return 0;
}

static int lua_wrapper_digitalWrite(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  int b = luaL_checkinteger(lua_state, 2);
  digitalWrite(a, b);
  return 0;
}

static int lua_wrapper_analogRead(lua_State *lua_state) {
  lua_pushnumber(lua_state, (lua_Number) analogRead(34));
  return 1;
}

#ifdef USE_EXPANDER
static int lua_wrapper_exp_pinMode(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  int b = luaL_checkinteger(lua_state, 2);
  expander.pinMode(a, b);
  return 0;
}

static int lua_wrapper_exp_digitalWrite(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  int b = luaL_checkinteger(lua_state, 2);
  expander.digitalWrite(a, b);
  return 0;
}

static int lua_wrapper_exp_digitalRead(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  lua_pushnumber(lua_state, (lua_Number) expander.digitalRead(a));
  return 1;
}
#endif

static int lua_wrapper_delay(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  delay(a);
  return 0;
}

void wait_command_sync(){
  while (sam_command_sync != SAMOVAR_NONE) delay(100);
}

static int lua_wrapper_millis(lua_State *lua_state) {
  lua_pushnumber(lua_state, (lua_Number) millis());
  return 1;
}

static int lua_wrapper_set_pause_withdrawal(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  pause_withdrawal(a);
  return 0;
}

static int lua_wrapper_set_power(lua_State *lua_state) {
  wait_command_sync();
  int a = luaL_checkinteger(lua_state, 1);

  if (a && !PowerOn) {
    if (Samovar_Mode == SAMOVAR_BEER_MODE && !PowerOn) {
      if (!PowerOn) sam_command_sync = SAMOVAR_BEER;
    } else if (Samovar_Mode == SAMOVAR_BK_MODE && !PowerOn) {
      if (!PowerOn) sam_command_sync = SAMOVAR_BK;
    } else if (Samovar_Mode == SAMOVAR_DISTILLATION_MODE && !PowerOn) {
      if (!PowerOn) sam_command_sync = SAMOVAR_DISTILLATION;
    } else
      sam_command_sync = SAMOVAR_POWER;
  }
   else if (!a && PowerOn)
     sam_command_sync = SAMOVAR_POWER;

  return 0;
}

static int lua_wrapper_set_mixer(lua_State *lua_state) {
  int a = luaL_checkinteger(lua_state, 1);
  set_mixer(a);
  return 0;
}

static int lua_wrapper_open_valve(lua_State *lua_state) {
  bool a = luaL_checkinteger(lua_state, 1);
  open_valve(a);
  return 0;
}

static int lua_wrapper_set_current_power(lua_State *lua_state) {
  float a = luaL_checknumber(lua_state, 1);
  set_current_power(a);
  return 0;
}

static int lua_wrapper_set_alarm(lua_State *lua_state) {
  set_alarm();
  return 0;
}

static int lua_wrapper_set_body_temp(lua_State *lua_state) {
  set_body_temp();
  return 0;
}

static int lua_wrapper_set_next_program(lua_State *lua_state) {
  wait_command_sync();
  if (Samovar_Mode == SAMOVAR_RECTIFICATION_MODE){
    sam_command_sync = SAMOVAR_START;
  } else if (Samovar_Mode == SAMOVAR_BEER_MODE){
    sam_command_sync = SAMOVAR_BEER_NEXT;
  }
  return 0;
}

static int lua_wrapper_get_state(lua_State *lua_state) {
  lua_pushnumber(lua_state, (lua_Number) SamovarStatusInt);
  return 1;
}

static int lua_wrapper_send_msg(lua_State *lua_state) {
  String st;
  const char *s;
  size_t l;
  int a = luaL_checkinteger(lua_state, 2);
  lua_getglobal(lua_state, "tostring");
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s = lua_tolstring(lua_state, -1, &l);
  st = s;
  lua_pop(lua_state, 1);
  if (a == -1) {
    WriteConsoleLog(st);
  } else {
    SendMsg(st, (MESSAGE_TYPE)a);
  }
  return 0;
}

static int lua_wrapper_set_num_variable(lua_State *lua_state){
  String Var;
  const char *s;
  size_t l;
  double a = luaL_checknumber(lua_state, 2);
  lua_getglobal(lua_state, "tostring");
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s = lua_tolstring(lua_state, -1, &l);
  Var = s;
  lua_pop(lua_state, 1);
  
    if (Var == "WFpulseCount") {
      WFpulseCount = (byte)a;
    } else if (Var == "pump_started") {
      pump_started = (int)a;
    } else if (Var == "valve_status") {
      valve_status = (int)a;
    } else if (Var == "SamSetup_Mode") {
      SamSetup.Mode = (int)a;
    } else if (Var == "Samovar_Mode") {
      Samovar_Mode = (SAMOVAR_MODE)a;
    } else if (Var == "Samovar_CR_Mode") {
      Samovar_CR_Mode = (SAMOVAR_MODE)a;
    } else if (Var == "acceleration_temp") {
      acceleration_temp = (uint16_t)a;
    } else if (Var == "wp_count") {
      wp_count = (byte)a;
    } else if (Var == "SteamTemp") {
      SteamSensor.avgTemp = a;
    } else if (Var == "PipeTemp") {
      PipeSensor.avgTemp = a;
    } else if (Var == "WaterTemp") {
      WaterSensor.avgTemp = a;
    } else if (Var == "TankTemp") {
      TankSensor.avgTemp = a;
    } else if (Var == "ACPTemp") {
      ACPSensor.avgTemp = a;
    } else if (Var == "loop_lua_fl") {
      loop_lua_fl = (int)a;
    } else if (Var == "show_lua_script") {
      show_lua_script = (int)a;
    } else if (Var == "test_num_val") {
      test_num_val = a;
    } else if (Var != "") {
      WriteConsoleLog("UNDEF NUMERIC LUA VAR " + Var + " = " + a);
    }
  return 0;
}

static int lua_wrapper_get_num_variable(lua_State *lua_state) {
  String Var;
  const char *s;
  size_t l;
  lua_getglobal(lua_state, "tostring");
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s = lua_tolstring(lua_state, -1, &l);
  Var = s;
  lua_pop(lua_state, 1);
  float a;
    if (Var == "WFpulseCount") {
      a = WFpulseCount;
    } else if (Var == "pump_started") {
      a = pump_started;
    } else if (Var == "valve_status") {
      a = valve_status;
    } else if (Var == "SamSetup_Mode") {
      a = SamSetup.Mode;
    } else if (Var == "Samovar_Mode") {
      a = Samovar_Mode;
    } else if (Var == "Samovar_CR_Mode") {
      a = Samovar_CR_Mode;
    } else if (Var == "acceleration_temp") {
      a = acceleration_temp;
    } else if (Var == "wp_count") {
      a = wp_count;
    } else if (Var == "SteamTemp") {
      a = SteamSensor.avgTemp;
    } else if (Var == "PipeTemp") {
      a = PipeSensor.avgTemp;
    } else if (Var == "WaterTemp") {
      a = WaterSensor.avgTemp;
    } else if (Var == "TankTemp") {
      a = TankSensor.avgTemp;
    } else if (Var == "ACPTemp") {
      a = ACPSensor.avgTemp;
    } else if (Var == "loop_lua_fl") {
      a = loop_lua_fl;
    } else if (Var == "show_lua_script") {
      a = show_lua_script;
    } else if (Var == "program_volume") {
      a = program[ProgramNum].Volume;
    } else if (Var == "program_speed") {
      a = program[ProgramNum].Speed;
    } else if (Var == "program_temp") {
      a = program[ProgramNum].Temp;
    } else if (Var == "program_power") {
      a = program[ProgramNum].Power;
    } else if (Var == "program_time") {
      a = program[ProgramNum].Time;
    } else if (Var == "program_capacity_num") {
      a = program[ProgramNum].capacity_num;
    } else if (Var == "test_num_val") {
      a = show_lua_script;
    } else if (Var != "") {
      WriteConsoleLog("SET UNDEF NUMERIC LUA VAR " + Var + " = " + a);
      return 0;
    }
  lua_pushnumber(lua_state, (lua_Number) a);
  return 1;
}


static int lua_wrapper_set_str_variable(lua_State *lua_state){
  String Var, Val;
  lua_getglobal(lua_state, "tostring");

  const char *s1;
  size_t l1;
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s1 = lua_tolstring(lua_state, -1, &l1);
  Var = s1;
  lua_pop(lua_state, 1);
  
  const char *s2;
  size_t l2;
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 2);
  lua_call(lua_state, 1, 1);
  s2 = lua_tolstring(lua_state, -1, &l2);
  Val = s2;
  lua_pop(lua_state, 1);

    if (Var == "Msg") {
      Msg = Val;
    } else if (Var == "SamovarStatus") {
      SamovarStatus = Val;
    } else if (Var == "test_str_val") {
      test_str_val = Val;
    } else if (Var == "program_type") {
      WriteConsoleLog("WARNING! program_type is read only property");
    } else if (Var != "") {
      WriteConsoleLog("UNDEF STRING LUA VAR " + Var + " = " + Val);
    }
  return 0;
}

static int lua_wrapper_set_object(lua_State *lua_state){
  String Var, Val;
  lua_getglobal(lua_state, "tostring");

  const char *s1;
  size_t l1;
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s1 = lua_tolstring(lua_state, -1, &l1);
  Var = s1;
  lua_pop(lua_state, 1);
  
  const char *s2;
  size_t l2;
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 2);
  lua_call(lua_state, 1, 1);
  s2 = lua_tolstring(lua_state, -1, &l2);
  Val = s2;
  lua_pop(lua_state, 1);

  luaObj->put(Var, Val);
  return 0;
}

static int lua_wrapper_get_object(lua_State *lua_state){
  String Var;
  const char *s;
  size_t l;
  lua_getglobal(lua_state, "tostring");
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s = lua_tolstring(lua_state, -1, &l);
  Var = s;
  lua_pop(lua_state, 1);
  lua_pushstring(lua_state, luaObj->get(Var).c_str());
  return 1;
}

static int lua_wrapper_set_timer(lua_State *lua_state) {
  byte a = luaL_checknumber(lua_state, 1);
  a--;
  if (a < 0 || a > 9) return 0;
  uint16_t b = luaL_checknumber(lua_state, 2);
  lua_timer[a] = millis() + b * 1000;
  return 0;
}

static int lua_wrapper_get_timer(lua_State *lua_state) {
  byte a = luaL_checknumber(lua_state, 1);
  uint16_t b;
  a--;
  if (a < 0 || a > 9) b = 0;
  else {
    if (lua_timer[a] == 0) b = 0;
    else {
      long l;
      l = lua_timer[a] - millis();
      if (l <= 0) b = 0;
      else b = l / 1000;
    }
  }
  lua_pushnumber(lua_state, (lua_Number) b);
  return 1;
}

static int lua_wrapper_get_str_variable(lua_State *lua_state){
  String Var;
  const char *s;
  size_t l;
  lua_getglobal(lua_state, "tostring");
  lua_pushvalue(lua_state, -1);
  lua_pushvalue(lua_state, 1);
  lua_call(lua_state, 1, 1);
  s = lua_tolstring(lua_state, -1, &l);
  Var = s;
  lua_pop(lua_state, 1);
  String c;
    if (Var == "Msg") {
      c = Msg;
    } else if (Var == "SamovarStatus") {
      c = SamovarStatus;
    } else if (Var == "test_str_val") {
      c = test_str_val;
    } else if (Var == "program_type") {
      c = program[ProgramNum].WType;
    } else if (Var != "") {
      WriteConsoleLog("UNDEF GET STRING LUA VAR " + Var + " = " + c);
      return 0;
    }
  lua_pushstring(lua_state, c.c_str());
  return 1;
}

void lua_init(){
  lua.Lua_register("pinMode", (const lua_CFunction) &lua_wrapper_pinMode);
  lua.Lua_register("digitalWrite", (const lua_CFunction) &lua_wrapper_digitalWrite);
  lua.Lua_register("analogRead", (const lua_CFunction) &lua_wrapper_analogRead);
#ifdef USE_EXPANDER
  lua.Lua_register("exp_pinMode", (const lua_CFunction) &lua_wrapper_exp_pinMode);
  lua.Lua_register("exp_digitalWrite", (const lua_CFunction) &lua_wrapper_exp_digitalWrite);
  lua.Lua_register("exp_digitalRead", (const lua_CFunction) &lua_wrapper_exp_digitalRead);
#endif
  lua.Lua_register("delay", (const lua_CFunction) &lua_wrapper_delay);
  lua.Lua_register("millis", (const lua_CFunction) &lua_wrapper_millis);  
  lua.Lua_register("sendMsg", (const lua_CFunction) &lua_wrapper_send_msg);

  lua.Lua_register("setPower", (const lua_CFunction) &lua_wrapper_set_power);
  lua.Lua_register("setBodyTemp", (const lua_CFunction) &lua_wrapper_set_body_temp);
  lua.Lua_register("setAlarm", (const lua_CFunction) &lua_wrapper_set_alarm);
  lua.Lua_register("setNumVariable", (const lua_CFunction) &lua_wrapper_set_num_variable);
  lua.Lua_register("setStrVariable", (const lua_CFunction) &lua_wrapper_set_str_variable);
  lua.Lua_register("setObject", (const lua_CFunction) &lua_wrapper_set_object);
  lua.Lua_register("setCurrentPower", (const lua_CFunction) &lua_wrapper_set_current_power);
  lua.Lua_register("setMixer", (const lua_CFunction) &lua_wrapper_set_mixer);
  lua.Lua_register("setNextProgram", (const lua_CFunction) &lua_wrapper_set_next_program);
  lua.Lua_register("setPauseWithdrawal", (const lua_CFunction) &lua_wrapper_set_pause_withdrawal);
  lua.Lua_register("setTimer", (const lua_CFunction) &lua_wrapper_set_timer);

  lua.Lua_register("openValve", (const lua_CFunction) &lua_wrapper_open_valve);

  lua.Lua_register("getNumVariable", (const lua_CFunction) &lua_wrapper_get_num_variable);
  lua.Lua_register("getStrVariable", (const lua_CFunction) &lua_wrapper_get_str_variable);
  lua.Lua_register("getState", (const lua_CFunction) &lua_wrapper_get_state);
  lua.Lua_register("getObject", (const lua_CFunction) &lua_wrapper_get_object);
  lua.Lua_register("getTimer", (const lua_CFunction) &lua_wrapper_get_timer);

  //Запускаем инициализирующий lua-скрипт
  File f = SPIFFS.open("/init.lua");
  if (f) {
    //нашли файл со скриптом, выполняем
    String script;
    script = get_global_variables();
    script += f.readString();
    f.close();
    if (show_lua_script) {
      WriteConsoleLog("-------BEGIN LUA SCRIPT-------");
      WriteConsoleLog(script);
      WriteConsoleLog("-------END LUA SCRIPT-------");
    }
    String sr = lua.Lua_dostring(&script);
    if (sr != "") WriteConsoleLog("init " + sr);
  }
  lua_type_script = get_lua_mode_name();
  lua_finished = true;

  glv = get_global_variables();
  load_lua_script();

  //Запускаем таск для запуска скрипта
  xTaskCreatePinnedToCore(
    do_lua_script,  /* Function to implement the task */
    "do_lua_script", /* Name of the task */
    10000,             /* Stack size in words */
    NULL,             /* Task input parameter */
    1,                /* Priority of the task */
    &DoLuaScriptTask,   /* Task handle. */
    0);               /* Core where the task should run */
}

String get_lua_script(bool type){
  String s;
  File f;
  if (!type){
    f = SPIFFS.open("/script.lua");
  } else {
    f = SPIFFS.open(lua_type_script);
  }
  if (f) {
    //нашли файл со скриптом, выполняем
    s = f.readString();
    f.close();
  }
  return s;
}

void load_lua_script(){
  script1 = get_lua_script(false).trim();
  script2 = get_lua_script(true).trim();
  if (script1 != "") script1 = glv + script1;
  if (script2 != "") script2 = glv + script2;
}

//Запускаем таск для запуска скрипта
void IRAM_ATTR do_lua_script(void *parameter) {
  String sr;
  while(1){
    if(!lua_finished){
      if (script1 != "") {
        if (show_lua_script) {
          WriteConsoleLog("-------BEGIN LUA SCRIPT-------");
          WriteConsoleLog(script1);
          WriteConsoleLog("-------END LUA SCRIPT-------");
        }
        sr = lua.Lua_dostring(&script1);
        if (sr != "") WriteConsoleLog(sr);
      }
      vTaskDelay(10/portTICK_PERIOD_MS);
    
      if (script2 != "") {
        if (show_lua_script) {
          WriteConsoleLog("-------BEGIN LUA SCRIPT-------");
          WriteConsoleLog(script2);
          WriteConsoleLog("-------END LUA SCRIPT-------");
        }
        sr = lua.Lua_dostring(&script2);
        if (sr != "") WriteConsoleLog(sr);
      }
      lua_finished = true;
    }
    delay (50);
  }
}

void start_lua_script(){
  if (!lua_finished) {
    WriteConsoleLog("lua runing");
    return;
  }
  
  lua_finished = false;
}

String get_global_variables(){
  String Variables;
//  Variables = "bme_temp = " + String(bme_temp) + "\r\n";
  Variables += "start_pressure = " + String(start_pressure) + "\r\n";
  Variables += "bme_pressure = " + String(bme_pressure) + "\r\n";
  Variables += "bme_prev_pressure = " + String(bme_prev_pressure) + "\r\n";
//  Variables += "bme_humidity = " + String(bme_humidity) + "\r\n";
//  Variables += "SamovarStatus = \"" + SamovarStatus + "\"\r\n";
  Variables += "capacity_num = " + String(capacity_num) + "\r\n";
  Variables += "SamovarStatusInt = " + String(SamovarStatusInt) + "\r\n";
//  Variables += "prev_ProgramNum = " + String(prev_ProgramNum) + "\r\n";
  Variables += "ProgramNum = " + String(ProgramNum) + "\r\n";
  Variables += "ProgramLen = " + String(ProgramLen) + "\r\n";
//  Variables += "startval = " + String(startval) + "\r\n";
//  Variables += "Delay1 = " + String(Delay1) + "\r\n";
//  Variables += "Delay2 = " + String(Delay2) + "\r\n";
  Variables += "currentvolume = " + String(currentvolume) + "\r\n";
  Variables += "currentstepcnt = " + String(currentstepcnt) + "\r\n";
//  Variables += "prev_time_ms = " + String(prev_time_ms) + "\r\n";
  Variables += "ActualVolumePerHour = " + String(ActualVolumePerHour) + "\r\n";
  Variables += "CurrrentStepperSpeed = " + String(CurrrentStepperSpeed) + "\r\n";
  Variables += "CurrrentStepps = " + String(CurrrentStepps) + "\r\n";
  Variables += "TargetStepps = " + String(TargetStepps) + "\r\n";
  Variables += "WthdrwlProgress = " + String(WthdrwlProgress) + "\r\n";
  Variables += "PowerOn = " + String(PowerOn) + "\r\n";
  Variables += "PauseOn = " + String(PauseOn) + "\r\n";
  Variables += "StepperMoving = " + String(StepperMoving) + "\r\n";
  Variables += "program_Pause = " + String(program_Pause) + "\r\n";
  Variables += "program_Wait = " + String(program_Wait) + "\r\n";
  Variables += "program_Wait_Type = \"" + program_Wait_Type + "\"\r\n";
//  Variables += "begintime = " + String(begintime) + "\r\n";
//  Variables += "t_min = " + String(t_min) + "\r\n";
//  Variables += "alarm_t_min = " + String(alarm_t_min) + "\r\n";
//  Variables += "alarm_h_min = " + String(alarm_h_min) + "\r\n";
  Variables += "WFpulseCount = " + String(WFpulseCount) + "\r\n";
  Variables += "WFflowMilliLitres = " + String(WFflowMilliLitres) + "\r\n";
  Variables += "WFtotalMilliLitres = " + String(WFtotalMilliLitres) + "\r\n";
  Variables += "WFflowRate = " + String(WFflowRate) + "\r\n";
  Variables += "WFAlarmCount = " + String(WFAlarmCount) + "\r\n";
  Variables += "acceleration_temp = " + String(acceleration_temp) + "\r\n";
  Variables += "WthdrwTimeAll = " + String(WthdrwTimeAll) + "\r\n";
  Variables += "WthdrwTime = " + String(WthdrwTime) + "\r\n";
  Variables += "WthdrwTimeAllS = \"" + WthdrwTimeAllS + "\"\r\n";
  Variables += "WthdrwTimeS = \"" + WthdrwTimeS + "\"\r\n";
  Variables += "pump_started = " + String(pump_started) + "\r\n";
  Variables += "setautospeed = " + String(setautospeed) + "\r\n";
  Variables += "heater_state = " + String(heater_state) + "\r\n";
//  Variables += "ofl = \"" + ofl + "\"\r\n";
  Variables += "mixer_status = " + String(mixer_status) + "\r\n";
//  Variables += "bk_pwm = " + String(bk_pwm) + "\r\n";
//  Variables += "chipId = " + String(chipId) + "\r\n";
  Variables += "alarm_event = " + String(alarm_event) + "\r\n";
  Variables += "acceleration_heater = " + String(acceleration_heater) + "\r\n";
  Variables += "valve_status = " + String(valve_status) + "\r\n";
  Variables += "program_type = \"" + program[ProgramNum].WType + "\"\r\n";
  Variables += "program_volume = " + String(program[ProgramNum].Volume) + "\r\n";
  Variables += "program_speed = " + String(program[ProgramNum].Speed) + "\r\n";
  Variables += "program_temp = " + String(program[ProgramNum].Temp) + "\r\n";
  Variables += "program_power = " + String(program[ProgramNum].Power) + "\r\n";
  Variables += "program_time = " + String(program[ProgramNum].Time) + "\r\n";
  Variables += "program_capacity_num = " + String(program[ProgramNum].capacity_num) + "\r\n";

//  Variables += "loop_lua_fl = " + String(loop_lua_fl) + "\r\n";
//  Variables += "show_lua_script = " + String(show_lua_script) + "\r\n";
  Variables += "SamSetup_Mode = " + String(SamSetup.Mode) + "\r\n";
  Variables += "test_num_val = " + String(test_num_val) + "\r\n";
  Variables += "test_str_val = \"" + test_str_val + "\"\r\n";

  Variables += "SteamTemp = " + String(SteamSensor.avgTemp) + "\r\n";
  Variables += "PipeTemp = " + String(PipeSensor.avgTemp) + "\r\n";
  Variables += "WaterTemp = " + String(WaterSensor.avgTemp) + "\r\n";
  Variables += "TankTemp = " + String(TankSensor.avgTemp) + "\r\n";
  Variables += "ACPTemp = " + String(ACPSensor.avgTemp) + "\r\n";

  Variables += "current_power_mode = \"" + current_power_mode + "\"\r\n";
  Variables += "target_power_volt = " + String(target_power_volt) + "\r\n";

#ifdef USE_WATER_PUMP
  Variables += "wp_count = " + String(wp_count) + "\r\n";
#endif
  
  return Variables;
}

String get_lua_mode_name(){
  String fl;
    if (Samovar_CR_Mode == SAMOVAR_BEER_MODE) {
      fl = "/beer.lua";
    } else if (Samovar_CR_Mode == SAMOVAR_DISTILLATION_MODE) {
      fl = "/dist.lua";
    } else if (Samovar_CR_Mode == SAMOVAR_BK_MODE) {
      fl = "/bk.lua";
    } else {
      fl = "/rectificat.lua";
    }
  return fl;
}
#endif
