#include "buzzer.h"

BuzzerESP32 buzzer(BUZZER_GPIO25);

// Non-blocking buzzer sequencer
struct ToneStep { uint16_t freq; unsigned long duration; unsigned long pauseAfter; };

// Medium tempo sequences
static const ToneStep mediumWelcome[] = {
  {523, 180, 200},
  {659, 180, 200},
  {784, 180, 0}
};
static const ToneStep mediumGranted[] = {
  {784, 160, 180},
  {987, 160, 0}
};
static const ToneStep mediumClosing[] = {
  {392, 220, 180},
  {349, 220, 0}
};
static const ToneStep mediumAlarm[] = {
  {1046, 300, 200},
  {1046, 300, 200},
  {1046, 300, 0}
};
static const ToneStep mediumWarning[] = {
  {330, 380, 180},
  {294, 380, 0}
};

static const ToneStep* buzzerSeq = nullptr;
static int buzzerSeqLen = 0;
static int buzzerIdx = 0;
enum BzState { BZ_IDLE, BZ_PLAYING, BZ_PAUSE };
static BzState bzState = BZ_IDLE;
static unsigned long bzNextMillis = 0;
// repeat support
static bool buzzerRepeat = false;
static unsigned long buzzerRepeatPauseMs = 0;

void buzzerPlaySequence(const void* seq, int len, bool repeat, unsigned long repeatPauseMs) {
  buzzerSeq = (const ToneStep*)seq;
  buzzerSeqLen = len;
  buzzerIdx = 0;
  buzzerRepeat = repeat;
  buzzerRepeatPauseMs = repeatPauseMs;
  bzState = BZ_PAUSE;
  bzNextMillis = millis();
}

void buzzerUpdate() {
  unsigned long now = millis();
  if (bzState == BZ_IDLE || buzzerSeqLen == 0) return;

  if (bzState == BZ_PAUSE) {
    if (now >= bzNextMillis) {
      if (buzzerIdx >= buzzerSeqLen) {
        if (buzzerRepeat) {
          // schedule next repetition after configured pause
          buzzerIdx = 0;
          bzState = BZ_PAUSE;
          bzNextMillis = now + buzzerRepeatPauseMs;
          buzzer.playTone(0,0);
          return;
        } else {
          bzState = BZ_IDLE;
          buzzer.playTone(0,0);
          return;
        }
      }
      buzzer.playTone(buzzerSeq[buzzerIdx].freq, 0); // start continuous tone
      bzState = BZ_PLAYING;
      bzNextMillis = now + buzzerSeq[buzzerIdx].duration;
    }
  } else if (bzState == BZ_PLAYING) {
    if (now >= bzNextMillis) {
      buzzer.playTone(0,0); // stop tone
      bzState = BZ_PAUSE;
      bzNextMillis = now + buzzerSeq[buzzerIdx].pauseAfter;
      buzzerIdx++;
    }
  }
}

// Convenience wrappers that trigger non-blocking sequences
void buzzerWelcome() { buzzerPlaySequence(mediumWelcome, sizeof(mediumWelcome)/sizeof(ToneStep)); }
void buzzerGrantedAccess() { buzzerPlaySequence(mediumGranted, sizeof(mediumGranted)/sizeof(ToneStep)); }
void buzzerClosing() { buzzerPlaySequence(mediumClosing, sizeof(mediumClosing)/sizeof(ToneStep)); }
void buzzerAlarmGas() { buzzerPlaySequence(mediumAlarm, sizeof(mediumAlarm)/sizeof(ToneStep), true, 5000UL); }
void buzzerWarningError() { buzzerPlaySequence(mediumWarning, sizeof(mediumWarning)/sizeof(ToneStep)); }

void buzzerOff() { buzzer.playTone(0,0); bzState = BZ_IDLE; buzzerSeq = nullptr; buzzerSeqLen = 0; }