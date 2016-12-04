/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(NICTA_GPL)
 */

#include <TimeClient.h>

void post_init()
{
    /* timeout once a second */
    int ret;
    ret = timer_periodic(0, 1000000000);
    printf("periodic timer started\n");
}

void timer_complete_callback()
{
    /* do nothing with the timer tick */
    printf("%s timer ticked\n", msg);
    printf("%s time is: %llu\n", msg, timer_time());
}
