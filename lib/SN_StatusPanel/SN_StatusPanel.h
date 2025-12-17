

void SN_StatusPanel__Init();

typedef enum {
  No_Color = 0,
  Solid_Blue,
  Solid_Green,
  Solid_Red,
  Solid_Yellow,
  Solid_XR4,
  Blink_Blue,
  Blink_Red,
  Blink_Green,
  Blink_Yellow,
  Blink_XR4,
  Moving_Back_Forth
} LED_State;

void SN_StatusPanel__SetStatusLedState(LED_State state);
void SN_StatusPanel__ControlHeadlights(bool on);