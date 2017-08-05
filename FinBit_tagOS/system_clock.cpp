/*
 * system_clock.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: dave
 */

/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== bigtime.cpp ========
 */

/* XDC module Headers */
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/System.h>

/* BIOS module Headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

/* Include system_clock.h for clock and time functions */
#include "system_clock.h"

#define TASKSTACKSIZE   512

const char *months[12] = { "January", "February", "March",
                           "April",   "May",      "June",
                           "July",    "August",   "September",
                           "October", "November", "December" };

Semaphore_Struct sem0Struct, sem1Struct;
Semaphore_Handle sem0Handle, sem1Handle;
Clock_Struct clk0Struct, clk1Struct;


/*
 * Clock methods
 */

Clock::Clock(int newId)
{
    id = newId;
    ticks = 0;
    microsecond = 0;
    millisecond = 0;
    second = 30;
    minute = 9;
    hour = 8;
    day = 31;
    month = 7;
    year = 17;
    century = 20;
    millenium = 0;
}

Clock::~Clock()
{
}

void Clock::tick()
{
    ticks++;

    if (getId() == 1) {             /* THIS IS ACTING AS A 4 MILLISECOND or 250 Hz CLOCK ROUTINE  */

        // System_printf("id %d : %d:%d:%d.%d\n", getId(), hour, minute, second, millisecond / 100);
        //System_printf("id %d : %s %d, %d%d\n", getId(), (IArg)months[month-1], day, century, year);
        /*
         * id 1 expires every 100 ticks (and each tick is 1 millisecond)
         */

        // Test lighting the blue LED every clock1 pass

        // GPIO_toggle(Board_LED0);                        // Toggle LED1 i.e.: the standalone RED LED
        // GPIO_toggle(Board_250HZ);                       // Toggle Pin 4.1 every 4ms
        // GPIO_write(Board_250HZ, 0);                     // Pulse Pin 4.1 low....
        // GPIO_write(Board_250HZ, 1);                     //       ... then high on every clock cycle; check it on oscilloscope

        // setMillisecond(0.5);             // originally 100 = 10Hz, so every minute; tested 4 = 25 Hz, trying 0.4 for 250Hz, or 0.5 for 200Hz
        setMillisecond(1);                  // try 1 millisecond ticks

    }
    if (getId() == 2) {             /* THIS IS ACTING AS A 1 SECOND or 1 Hz CLOCK ROUTINE, BEFORE BIOS_exit AFTER 2 MINUTES */

        // System_printf("\tid %d : %d:%d:%d\n", getId(), hour, minute, second);
        // System_printf("id %d : %s %d, %d%d\n", getId(), (IArg)months[month-1], day, century, year);

        /*
         * Change selected function to alter clock rate
         */

//      setMicrosecond();
//      setMillisecond();
//      setSecond();                    // Originally this was enabled to make this clock run in units of seconds
        setMinute();                    // Now setting for one minute as a potential burn after x hours y minutes test
//      setHour();
//      setDay();
        if (ticks == 2) {              // Originally this was set to run for 2 seconds, now minute timer, which runs for 120 seconds
            clockTerminate(0);
        }
    }

    return;
}

void Clock::setMicrosecond()
{
    if (microsecond >= 999) {
        setMillisecond();
        microsecond = 0;
    }
    else {
        microsecond++;
    }

    return;
}

void Clock::setMillisecond()
{
    if (millisecond >= 999) {
        setSecond();
        millisecond = 0;
    }
    else {
        millisecond++;
    }

    return;
}

void Clock::setMillisecond(int nMilliseconds)
{
    int secs;

    millisecond += nMilliseconds;
    secs = millisecond / 1000;
    millisecond %= 1000;

    while (secs--) {
        setSecond();
    }

    return;
}

void Clock::setSecond()
{
    if (second == 59) {
        setMinute();
        second = 0;
    }
    else {
        second++;
    }

    return;
}

void Clock::setMinute()
{
    if (minute == 59) {
        setHour();
        minute = 0;
    }
    else {
        minute++;
    }

    return;
}

void Clock::setHour()
{
    if (hour == 23) {
        setDay();
        hour = 0;
    }
    else {
        hour++;
    }

    return;
}

void Clock::setDay()
{
    bool thirtydays = false;
    bool feb = false;
    bool leap = false;

    if (month == 4 || month == 6 || month == 9 || month == 11) {
        // April, June, September, November.
        thirtydays = true;
    }

    if (month == 2) {  // Test for February
        feb = true;
    }

    /*
     * A year is a leap year if it is divisible by 4, but not by 100.
     *
     * If a year is divisible by 4 and by 100, it is a leap year only
     * if it is also divisible by 400.
     */
    if ((year%4 == 0 && year%100 != 0) ||
            (year%4 == 0 && year%100 == 0 && year%400 == 0)) {
        leap = true;
    }

    if ((day == 28) && (feb) && (!leap)) {
        setMonth();
        day = 1;
    }
    else if ((day == 29) && (feb) && (leap)) {
        setMonth();
        day = 1;
    }
    else if ((day == 30) && (thirtydays == true)) {
        setMonth();
        day = 1;
    }
    else if ((day == 31) && (thirtydays == false)) {
        setMonth();
        day = 1;
    }
    else {
        day++;
    }

    return;
}

void Clock::setMonth()
{
    if (month >= 12) {
        setYear();
        month = 1;
    }
    else {
        month++;
    }

    return;
}

void Clock::setYear()
{
    year++;
    if ((year%100) == 0) {
        setCentury();
    }

    return;
}

void Clock::setCentury()
{
    century++;
    if ((century%10) == 0) {
        setMillenium();
    }

    return;
}

void Clock::setMillenium()
{
    millenium++;

    return;
}

long Clock::getTicks()
{
    return ticks;
}

int Clock::getId()
{
    return id;
}

int Clock::getMicrosecond()
{
    return microsecond;
}

int Clock::getMillisecond()
{
    return millisecond;
}

int Clock::getSecond()
{
    return second;
}

int Clock::getMinute()
{
    return minute;
}

int Clock::getHour()
{
    return hour;
}

int Clock::getDay()
{
    return day;
}

int Clock::getMonth()
{
    return month;
}

int Clock::getYear()
{
    return year;
}

int Clock::getCentury()
{
    return century;
}

int Clock::getMillenium()
{
    return millenium;
}




/*
 *  ======== clockTerminate ========
 *  This function simply terminates the example
 */
void clockTerminate(UArg arg)
{
    Clock *clock = (Clock *)arg;

    System_printf("Heading for BIOS_exit after %d clockTicks (ms) of execution.\n", clock->getMillisecond());

    if (GPIO_read(Board_LED0)) {
        GPIO_write(Board_LED0, Board_LED_OFF);              // Turn off the standalone red LED if it's turned on, to indicate BIOS_exit
    }

    if (GPIO_read(Board_LED2)) {
        GPIO_write(Board_LED2, Board_LED_OFF);              // Turn off green LED to indicate BIOS_exit
    }

    Task_sleep(1000);                                       // Turn on blue LED for one second to indicate BIOS exit completion
    GPIO_write(Board_LED3, Board_LED_ON);
    Task_sleep(1000);
    GPIO_write(Board_LED3, Board_LED_OFF);

    BIOS_exit(0);

}

/*
 *  ======== clockTask ========
 *  Wrapper function for Task objects calling
 *  Clock::tick()
 */
void clockTask(UArg arg)
{
    Clock *clock = (Clock *)arg;
    int count = 0;

    if (clock->getId() == 3) {
        for(;;) {             // task id = 3
            Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);
            clock->tick();
            if(count == 50) {
                Task_sleep(25);
                count = 0;
            }
            count++;
            Semaphore_post(sem1Handle);
        }
    }
    else {
        for(;;) {             // task id = 4
            Semaphore_pend(sem1Handle, BIOS_WAIT_FOREVER);
            if(count == 50) {
                Task_sleep(25);
                count = 0;
            }
            clock->tick();
            count++;
            Semaphore_post(sem0Handle);
        }
    }
}


/*
 * ======== clockPrd ========
 * Wrapper function for PRD objects calling
 * Clock::tick()
 */

void clockPrd(UArg arg)
{
    Clock *clock = (Clock *)arg;

    clock->tick();
    return;
}

