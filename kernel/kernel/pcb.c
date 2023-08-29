#include "common.h"
#include "common/pool.h"
#include "x86.h"
#include <stdint.h>

extern ProcessTable pcb[MAX_PCB_NUM]; // pcb
proc pcb_pts[MAX_PCB_NUM];            // pointers

pool _busy;
pool _avai;
pool _sleep;
pool _zombie;

pool *busy = &_busy;
pool *avai = &_avai;
pool *slp = &_sleep;
pool *zombie = &_zombie;

proc _bh, _bt, _ah, _at, _sh, _st, _zh, _zt;