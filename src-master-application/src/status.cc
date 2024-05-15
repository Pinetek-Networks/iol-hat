#include "osal.h"
#include "osal_log.h"
#include "iolink.h"
#include "iolink_smi.h"
#include "iolink_max14819.h"
#include "iolink_pl.h"

#include "status.h"
#include "common.h"

#include <unistd.h>

uint16_t pdCount0, pdCount1;


void *runStatus (void *_arg)
{	
	uint16_t myTime0;
	uint16_t myTime1;
	
	pdCount0 = 0;
	pdCount1 = 0;
	
  // Start Status thread Server
	while (1)
	{
		
		
		myTime0= 1000/pdCount0;
		myTime1= 1000/pdCount1;
		
		pdCount0 = 0;
		pdCount1 = 0;
		
		
		LOG_INFO (IOLINK_PL_LOG, "Cycle time chn #1=%d ms, chn #2=%d ms\n", myTime0, myTime1);
		
		sleep(1);
	}
}