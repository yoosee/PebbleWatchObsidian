#pragma once

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

#define KEY_IS_FAHRENHEIT 2

#define KEY_COLOR_BACKGROUND 10
#define KEY_COLOR_FACE 11
#define KEY_COLOR_STEPS 12
#define KEY_COLOR_WEATHER 13
#define KEY_COLOR_HOURHAND 14
#define KEY_COLOR_MINUTEHAND 15

extern void init();
extern void deinit();

// Watch Configuration resources

#define NUM_CLOCK_TICKS 8

#define TICK_UPDATE_SECONDS 30

#define COLOR_DEFAULT_BACKGROUND 0x000000
#define COLOR_DEFAULT_FACE 0xffffff
#define COLOR_DEFAULT_STEPS 0x482748
#define COLOR_DEFAULT_WEATHER 0x4cb4db
#define COLOR_DEFAULT_HOURHAND 0xe35462
#define COLOR_DEFAULT_MINUTEHAND 0xffffff

static const struct GPathInfo ANALOG_BG_POINTS[] = {
  { 4, (GPoint []){
      {112, 10},
      {114, 12},
      {108, 23},
      {106, 21}
    }
  },
  { 4, (GPoint []){
      {132, 47},
      {144, 40},
      {144, 44},
      {135, 49}
    }
  },
  { 4, (GPoint []){
      {135, 118},
      {144, 123},
      {144, 126},
      {132, 120}
    }
  },
  { 4, (GPoint []){
      {108, 144},
      {114, 154},
      {112, 157},
      {106, 147}
    }
  },

  { 4, (GPoint []){
      {32, 10},
      {30, 12},
      {36, 23},
      {38, 21}
    }
  },
  { 4, (GPoint []){
      {12, 47},
      {-1, 40},
      {-1, 44},
      {9, 49}
    }
  },
  { 4, (GPoint []){
      {9, 118},
      {-1, 123},
      {-1, 126},
      {12, 120}
    }
  },
  { 4, (GPoint []){
      {36, 144},
      {30, 154},
      {32, 157},
      {38, 147}
    }
  },
};

static const GPathInfo MINUTE_HAND_POINTS = {
  8, (GPoint []) {
    { -5, 10 },
    { -3, 14 },
    {  3, 14 },
    {  5, 10 },
    {  5,-76 },
    {  3,-80 },
    { -3,-80 },
    { -5,-76 },  
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  8, (GPoint []){
    { -6,   6 },
    { -4, 10 },
    {  4, 10 },
    {  6,   6 },
    {  6,-53 },
    {  4,-60 },
    { -4,-60 },
    { -6,-53 },  
  }
};