#pragma once

#include <lwip/timeouts.h>

/*@
  @ requires \valid_function(handler);
  @ assigns \nothing;
  @*/
void sys_timeout(u32_t msecs, sys_timeout_handler handler, void *arg);

/*@
  @ requires \valid_function(handler);
  @ assigns \nothing;
  @*/
void sys_untimeout(sys_timeout_handler handler, void *arg);
