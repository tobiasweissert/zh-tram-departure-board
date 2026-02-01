#include "Utils.h"
#include "Config.h"
#include <Preferences.h>
#include <string.h>

Preferences preferences;

unsigned long getCurrentTimestamp() {
  return time(nullptr);
}

void getCurrentTimeString(char* buffer, size_t bufferSize) {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  strftime(buffer, bufferSize, "%H:%M", timeinfo);
}

void convertUmlauts(char* text, size_t bufferSize) {
  // Work in-place by using a temporary buffer
  char temp[64];
  int bufIdx = 0;
  int textLen = strlen(text);

  // Prevent buffer overflow
  if (textLen > 63) textLen = 63;

  for (int i = 0; i < textLen && bufIdx < 63; i++) {
    unsigned char c = (unsigned char)text[i];

    if (c == 0xC3 && i + 1 < textLen) {
      unsigned char next = (unsigned char)text[i + 1];
      i++;

      switch (next) {
        case 0xA4: temp[bufIdx++] = (char)132; break;  // ä
        case 0xB6: temp[bufIdx++] = (char)148; break;  // ö
        case 0xBC: temp[bufIdx++] = (char)129; break;  // ü
        case 0x84: temp[bufIdx++] = (char)142; break;  // Ä
        case 0x96: temp[bufIdx++] = (char)153; break;  // Ö
        case 0x9C: temp[bufIdx++] = (char)154; break;  // Ü
        case 0x9F: temp[bufIdx++] = (char)225; break;  // ß
        default: temp[bufIdx++] = '?'; break;
      }
    } else {
      temp[bufIdx++] = (char)c;
    }
  }

  temp[bufIdx] = '\0';

  // Copy result back to original buffer
  strncpy(text, temp, bufferSize - 1);
  text[bufferSize - 1] = '\0';
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
