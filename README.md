# Estaciones_químicas

Sistema de medicion de variables para análisis con aplicaciones químicas. El objetivo es desarrollar un sistema que permita medir variables químicas, (ej NO3-, F- y pH) en matraces con líquidos y muestras de suelos.

- Código realizado en Platformio
- Microcontrolador basado en Arduino Uno

## Sondas a integrar:
- Sensor pH x041800394 (Gravity - Analog Spear) https://www.dfrobot.com/product-1668.html
- Sensor Oxigeno Disuelto (Atlas scientific) 
- Sensor pH4502c
- Sensor Oxigeno discuelto dog2 - A0007 (Gravity)
- Nitrato NO3 (tipo serie 43) serie NO43 - A0001

## Requerimientos del sistema

- Microcnotrolador.
- Datalogger
- Tipo de comunicación
- Códigos que permitan calibración online.

## Dependencias
- ph_AO Analog   ph sensor gravity industrial
- DO_RX Software serial DO 
- DO_TX Software serial DO 
- ph_A1 Analog   ph sensor 
- DO_A2 Analog   DO sensor gravity
- ph_A3 Analog   ph sensor gravity lab
- phOPR_A4 Analog ph/opr sensor


## TODO