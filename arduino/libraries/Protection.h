#ifndef Protection_h
#define Protection_h
#include <Arduino.h>
enum STATE { STATE_INIT, STATE_ARMED, STATE_PICKUP, STATE_TRIPPED};
#define MS_TO_SETTLE 500

class Protection{
  public:
    Protection();
    Protection(int limit, float T, byte pin, char * name);
    void run(int sample);
    void reset();

  private:
    uint8_t _pin;
    int _limit;
    void trip();
    unsigned long iec_curve(long sample);
    float k;
    float alpha;
    float beta;
    float _T;
    STATE state;
    String _name;
    unsigned long t_pickup = 0;
    unsigned long t_trip = 0;
    unsigned long t_settled = 0;
};

#endif