#ifndef _LAPIC_H
#define _LAPIC_H

void lapicinit(void);
void lapicstartap(uchar apicid, uint addr);
int cpunum(void);
void stop_other_cpus(void);

extern volatile uint *lapic;

#endif
