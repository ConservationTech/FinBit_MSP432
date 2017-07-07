/*
 * system_clock.h
 *
 *  Created on: Jun 29, 2017
 *      Author: dave
 */

#ifndef SYSTEM_CLOCK_H_
#define SYSTEM_CLOCK_H_

class Clock {
    private:
         // data
         int id;
         double ticks;
         int microsecond;
         int millisecond;
         int second;
         int minute;
         int hour;
         int day;
         int month;
         int year;
         int century;
         int millenium;
         Diags_Mask clockLog;

    public:
        // methods
        Clock(int newId);  // Constructor
        ~Clock();          // Destructor
        void tick();
        long getTicks();
        int getId();
        int getMicrosecond();
        int getMillisecond();
        int getSecond();
        int getMinute();
        int getHour();
        int getDay();
        int getMonth();
        int getYear();
        int getCentury();
        int getMillenium();
        void setMicrosecond();
        void setMillisecond();
        void setMillisecond(int nMilliseconds);
        void setSecond();
        void setMinute();
        void setHour();
        void setDay();
        void setMonth();
        void setYear();
        void setCentury();
        void setMillenium();
};

void clockTerminate(UArg arg);

/* Wrapper functions to call Clock::tick() */
void clockTask(UArg arg);
void clockPrd(UArg arg);
// void clockIdle(void);

#endif /* SYSTEM_CLOCK_H_ */
