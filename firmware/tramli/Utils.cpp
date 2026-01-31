#include "Utils.h"
#include "Config.h"
#include <Preferences.h>

Preferences preferences;

unsigned long getCurrentTimestamp() {
  return time(nullptr);
}

String getCurrentTimeString() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
  return String(buffer);
}

String convertUmlauts(String text) {
  // Pre-allocate buffer to avoid heap fragmentation
  char buffer[64];
  int bufIdx = 0;
  int textLen = text.length();

  // Prevent buffer overflow
  if (textLen > 63) textLen = 63;

  for (int i = 0; i < textLen && bufIdx < 63; i++) {
    unsigned char c = text[i];

    if (c == 0xC3 && i + 1 < textLen) {
      unsigned char next = text[i + 1];
      i++;

      switch (next) {
        case 0xA4: buffer[bufIdx++] = (char)132; break;
        case 0xB6: buffer[bufIdx++] = (char)148; break;
        case 0xBC: buffer[bufIdx++] = (char)129; break;
        case 0x84: buffer[bufIdx++] = (char)142; break;
        case 0x96: buffer[bufIdx++] = (char)153; break;
        case 0x9C: buffer[bufIdx++] = (char)154; break;
        case 0x9F: buffer[bufIdx++] = (char)225; break;
        default: buffer[bufIdx++] = '?'; break;
      }
    } else {
      buffer[bufIdx++] = (char)c;
    }
  }

  buffer[bufIdx] = '\0';
  return String(buffer);
}

void initNVS() {
  preferences.begin(NVS_NAMESPACE, false);
}

void saveTimeToNVS(time_t timestamp) {
  preferences.putUInt("timestamp", (uint32_t)timestamp);
}

time_t loadTimeFromNVS() {
  return (time_t)preferences.getUInt("timestamp", 0);
}
