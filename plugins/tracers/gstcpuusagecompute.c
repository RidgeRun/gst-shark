/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
 *
 * This file is part of GstShark.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/gst.h>
#include <glib/gstdio.h>
#include "gstcpuusage.h"
#include "gstcpuusagecompute.h"

#include <unistd.h>
#include <string.h>


void
gst_cpu_usage_init (GstCPUUsage * cpuusage)
{
  gint32 cpu_num;

  g_return_if_fail (cpuusage);

  memset (cpuusage, 0, sizeof (GstCPUUsage));

  if ((cpu_num = sysconf (_SC_NPROCESSORS_CONF)) == -1) {
    GST_WARNING ("failed to get number of cpus");
    cpu_num = 1;
  }

  cpuusage->cpu_num = cpu_num;
}

void
gst_cpu_usage_compute (GstCPUUsage * cpuusage)
{
  gdouble *cpu_usage;
  gint32 cpu_num;
  gint cpu_id;
  FILE *fd;

  gint *user;
  gint *user_aux;
  gint *nice;
  gint *nice_aux;
  gint *system;
  gint *system_aux;
  gint *idle;
  gint *idle_aux;

  gint iowait;                  /* Time waiting for I/O to complete */
  gint irq;                     /* Time servicing interrupts        */
  gint softirq;                 /* Time servicing softirqs          */
  gint steal;                   /* Time spent in other OSes when in virtualized env */
  gint quest;                   /* Time spent running a virtual CPU for guest OS    */
  gint quest_nice;              /* Time spent running niced guest */
  gdouble num_value;
  gdouble den_value;
  gboolean cpu_array_sel;
  gint ret;

  g_return_if_fail (cpuusage);

  user = cpuusage->user;
  user_aux = cpuusage->user_aux;
  nice = cpuusage->nice;
  nice_aux = cpuusage->nice_aux;
  system = cpuusage->system;
  system_aux = cpuusage->system_aux;
  idle = cpuusage->idle;
  idle_aux = cpuusage->idle_aux;

  cpu_array_sel = cpuusage->cpu_array_sel;
  cpu_usage = cpuusage->cpu_usage;
  cpu_num = cpuusage->cpu_num;



  /* Compute the load for each core */
  fd = g_fopen ("/proc/stat", "r");
  if (cpu_array_sel == 0) {
    ret =
        fscanf (fd, "%*s %d %d %d %d %d %d %d %d %d %d", &user[0], &nice[0],
        &system[0], &idle[0], &iowait, &irq, &softirq, &steal, &quest,
        &quest_nice);
    for (cpu_id = 0; cpu_id < cpu_num; ++cpu_id) {
      ret =
          fscanf (fd, "%*s %d %d %d %d %d %d %d %d %d %d", &user[cpu_id],
          &nice[cpu_id], &system[cpu_id], &idle[cpu_id], &iowait, &irq,
          &softirq, &steal, &quest, &quest_nice);
    }
    /* Compute the utilization for each core */
    for (cpu_id = 0; cpu_id < cpu_num; ++cpu_id) {
      num_value =
          ((user[cpu_id] + nice[cpu_id] + system[cpu_id]) - (user_aux[cpu_id] +
              nice_aux[cpu_id] + system_aux[cpu_id]));
      den_value =
          ((user[cpu_id] + nice[cpu_id] + system[cpu_id] + idle[cpu_id]) -
          (user_aux[cpu_id] + nice_aux[cpu_id] + system_aux[cpu_id] +
              idle_aux[cpu_id]));
      cpu_usage[cpu_id] = num_value / den_value;
    }
    cpu_array_sel = 1;
  } else {
    ret =
        fscanf (fd, "%*s %d %d %d %d %d %d %d %d %d %d", &user_aux[0],
        &nice_aux[0], &system_aux[0], &idle_aux[0], &iowait, &irq, &softirq,
        &steal, &quest, &quest_nice);
    for (cpu_id = 0; cpu_id < cpu_num; ++cpu_id) {
      ret =
          fscanf (fd, "%*s %d %d %d %d %d %d %d %d %d %d", &user_aux[cpu_id],
          &nice_aux[cpu_id], &system_aux[cpu_id], &idle_aux[cpu_id], &iowait,
          &irq, &softirq, &steal, &quest, &quest_nice);
    }
    /* Compute the utilization for each core */
    for (cpu_id = 0; cpu_id < cpu_num; ++cpu_id) {
      num_value =
          ((user_aux[cpu_id] + nice_aux[cpu_id] + system_aux[cpu_id]) -
          (user[cpu_id] + nice[cpu_id] + system[cpu_id]));
      den_value =
          ((user_aux[cpu_id] + nice_aux[cpu_id] + system_aux[cpu_id] +
              idle_aux[cpu_id]) - (user[cpu_id] + nice[cpu_id] +
              system[cpu_id] + idle[cpu_id]));
      cpu_usage[cpu_id] = num_value / den_value;
    }
    cpu_array_sel = 0;
  }

  (void) ret;

  cpuusage->cpu_array_sel = cpu_array_sel;
  fclose (fd);
}
