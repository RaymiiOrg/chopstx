/*
 * eventflag.c - Eventflag with timeout
 *
 * Copyright (C) 2013 Flying Stone Technology
 * Author: NIIBE Yutaka <gniibe@fsij.org>
 *
 * This file is a part of Chopstx, a thread library for embedded.
 *
 * Chopstx is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Chopstx is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As additional permission under GNU GPL version 3 section 7, you may
 * distribute non-source form of the Program without the copy of the
 * GNU GPL normally required by section 4, provided you inform the
 * receipents of GNU GPL by a written offer.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <chopstx.h>
#include <eventflag.h>

void
eventflag_init (struct event_flag *ev, chopstx_t owner)
{
  ev->owner = owner;
  ev->wait_usec = 0;
  ev->flag = 0;
  chopstx_mutex_init (&ev->mutex);
}


eventmask_t
eventflag_wait_timeout (struct event_flag *ev, uint32_t usec)
{
  eventmask_t em = 0;
  int n;

  chopstx_mutex_lock (&ev->mutex);

  if (!ev->flag)
    {
      ev->wait_usec = usec;
      chopstx_mutex_unlock (&ev->mutex);
      chopstx_usec_wait_var (&ev->wait_usec);
      chopstx_mutex_lock (&ev->mutex);
      ev->wait_usec = 0;
    }

  n = __builtin_ffs (ev->flag);
  if (n)
    {
      em = (1 << (n - 1));
      ev->flag &= ~em;
    }

  chopstx_mutex_unlock (&ev->mutex);
  return em;
}


void
eventflag_signal (struct event_flag *ev, eventmask_t m)
{
  chopstx_mutex_lock (&ev->mutex);
  ev->flag |= m;
  if (ev->wait_usec)
    {
      ev->wait_usec = 0;
      chopstx_wakeup_usec_wait (ev->owner);
    }
  chopstx_mutex_unlock (&ev->mutex);
}
