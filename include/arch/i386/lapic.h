#ifndef _LAPIC_H
#define _LAPIC_H

void lapicinit(void);
void lapicstartap(uchar apicid, uint addr);
int cpunum(void);

extern volatile uint *lapic;

#endif
