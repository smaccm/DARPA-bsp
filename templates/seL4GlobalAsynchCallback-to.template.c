/*#
 *# Copyright 2014, NICTA
 *#
 *# This software may be distributed and modified according to the terms of
 *# the BSD 2-Clause license. Note that NO WARRANTY is provided.
 *# See "LICENSE_BSD2.txt" for details.
 *#
 *# @TAG(NICTA_BSD)
 #*/

#include <assert.h>
#include <sel4/sel4.h>
#include <stddef.h>

/*? macros.show_includes(me.to_instance.type.includes) ?*/

/*- set is_reader = True -*/
/*- set instance = me.to_instance.name -*/
/*- set interface = me.to_interface.name -*/
/*- include 'global-endpoint.template.c' -*/
/*- set notification = pop('notification') -*/

void /*? me.to_interface.name ?*/_callback(void);

void /*? me.to_interface.name ?*/__run(void) {
    while(1) {
        seL4_Wait(/*? notification ?*/, NULL);
        /*? me.to_interface.name ?*/_callback();
    }
}

seL4_CPtr /*? me.to_interface.name ?*/_notification(void) {
    return /*? notification ?*/;
}
