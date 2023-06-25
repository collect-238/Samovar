--Скрипт для контроля температуры брожения

--НАЧАЛЬНЫЕ УСТАНОВКИ
T = 19 -- температура, ниже которой скрипт не работает

-- ОПРЕДЕЛЕНИЕ ПЕРЕМЕННЫХ
TankTemp = getNumVariable("TankTemp") + 0
ACPTemp = getNumVariable("ACPTemp") + 0
ValveStatus = getNumVariable("valve_status") + 0

if TankTemp < T and ValveStatus == 1 then
   openValve(0)
else
   if ACPTemp + 0.5 < TankTemp and ValveStatus == 0 then
      openValve(1)
   end

   if ACPTemp - 0.5 > TankTemp and ValveStatus == 1 then
      openValve(0)
   end

end

ValveStatus = getNumVariable("valve_status") + 0

status = string.format("ACPT = %.2f; TankT = %.2f; Клапан %.0f", ACPTemp, TankTemp, ValveStatus)

setLuaStatus(status)
--sendMsg(status, 2) --пишем оператору