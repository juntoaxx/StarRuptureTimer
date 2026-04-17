#pragma once

#include "plugin_interface.h"

IPluginSelf* GetSelf();

inline IPluginHooks*   GetHooks()   { auto* s = GetSelf(); return s ? s->hooks   : nullptr; }
inline IPluginConfig*  GetConfig()  { auto* s = GetSelf(); return s ? s->config  : nullptr; }
inline IPluginScanner* GetScanner() { auto* s = GetSelf(); return s ? s->scanner : nullptr; }

#define LOG_TRACE(fmt, ...) do { if (auto* _s = GetSelf()) _s->logger->Trace(_s, fmt, ##__VA_ARGS__); } while(0)
#define LOG_DEBUG(fmt, ...) do { if (auto* _s = GetSelf()) _s->logger->Debug(_s, fmt, ##__VA_ARGS__); } while(0)
#define LOG_INFO(fmt, ...)  do { if (auto* _s = GetSelf()) _s->logger->Info (_s, fmt, ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...)  do { if (auto* _s = GetSelf()) _s->logger->Warn (_s, fmt, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(fmt, ...) do { if (auto* _s = GetSelf()) _s->logger->Error(_s, fmt, ##__VA_ARGS__); } while(0)
