//Criado por Bigujun em 02/2018
#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#define DEBUG_SMART true

  #ifdef DEBUG_SMART
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
    #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_PRINTF(...)  Serial.printf(__VA_ARGS__)
    #define DEBUG_BEGIN(...) Serial.begin(__VA_ARGS__);
  #else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
    #define DEBUG_PRINTF(...)
    #define DEBUG_BEGIN(...)
  #endif

#endif
