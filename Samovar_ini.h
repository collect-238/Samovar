//**************************************************************************************************************
// Параметры подключения к WIFI
//**************************************************************************************************************
#define SAMOVAR_HOST "samovar"
#define SAMOVAR_SSID "ssid"
#define SAMOVAR_PASSWORD "pwd"
#define SAMOVAR_AUTH "auth-auth"
//**************************************************************************************************************

//**************************************************************************************************************
// Настройки предельных значений для контроля автоматики
//**************************************************************************************************************
//Температура воды, при достижении которой будет оповещен оператор
#define ALARM_WATER_TEMP 70
//Максимальное значение температуры воды, при котором выключится питание
#define MAX_WATER_TEMP 75
//Максимальное значение температуры пара, при котором выключится питание
#define MAX_STEAM_TEMP 99.8
//Значение температуры датчика пара, при котором колонна перейдет из режима разгона в рабочий режим
#define CHANGE_POWER_MODE_STEAM_TEMP 76.1
//Значение температуры датчика в кубе, при котором откроется клапан подачи воды
#define OPEN_VALVE_TANK_TEMP 59.5
//Сопротивление тэна для расчета мощности
#define HEATER_RESISTANT 15.2
//**************************************************************************************************************

//**************************************************************************************************************
// Настройки дополнительных параметров
//**************************************************************************************************************
#define SAMOVAR_LOG_PERIOD 3                // периодичность записи данных о температуре в файл (раз в три секунды, оптимально с точки зрения объема файла)
#define TIMEZONE 3                          // таймзона того места, где будет применяться устройство
//**************************************************************************************************************

//**************************************************************************************************************
// Настройки использования подсистем Самовара
//**************************************************************************************************************
#define SAMOVAR_USE_BLYNK                   //использовать Blynk в проекте
#define SAMOVAR_USE_POWER                   //использовать регулятор напряжения в проекте
#define USE_WATERSENSOR                     //использовать датчик потока воды охлаждения. Если используется - в случае отсутствия воды в течении заданного времени Самовар отключит нагрев колонны
#define USE_HEAD_LEVEL_SENSOR               //использовать датчик уровня флегмы в голове. Если используется - в случае срабатывания датчика, снизит напряжение на 3 вольта

//#define USE_UPDATE_OTA                      //использовать ArduinoOTA для обновления через WI-FI. Удобно при отладке, при работе создает лишнюю нагрузку на ESP32
#define USE_PRESSURE_CORRECT                // использовать корректировку температуры отбора тела при изменении давления

#define WRITE_PROGNUM_IN_LOG                // писать в лог номер текущей строки программы
