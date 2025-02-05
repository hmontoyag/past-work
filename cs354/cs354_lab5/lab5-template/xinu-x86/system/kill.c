/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
    // LAB2: TODO: Modify this function to clean-up the pipe.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

	/*789*/
	/*pipdelete associated pipe*/
	struct pipe_t *pipeptr;
	 for(int i = 0; i<MAXPIPES; i++){
    	pipeptr = &pipe_tables[i];
    	
    	//thi pid is a writer
    	if(pipeptr->writerpid == pid){
    		//kprintf("kill pipe: %d<><><write><><%d\n",pipeptr->pipid,pid);
    		pipeptr->removingwriter = 1;
    		pipeptr->writerpid = -1;
    		pipeptr->wpos = 0;
			struct dentry *devptr;
			devptr = &devtab[pipeptr->bounddev];
			
    		if(semcount(pipeptr->rsem)!=0){
    			//kprintf("semcount ni kill\n");
    			while(semcount(pipeptr->rsem)!=0){
    				//kprintf("w\n");
    				pipgetc(devptr);
    			}
    		}
			    
			pipeptr->readerpid = -1;
			pipeptr->state = PIPE_USED;
			pipeptr->bounddev = -1;
			semreset(pipeptr->wsem, PIPE_SIZE);
			semreset(pipeptr->rsem, 0);
			pipeptr->removingwriter = 0;
			
    
    	}	
    	
    	//this pid is a reader
    	if(pipeptr->readerpid == pid){    		
    		//kprintf("kill pipe: %d<><><read><><%d\n",pipeptr->pipid,pid);
    	  	pipeptr->writerpid = -1;
    		pipeptr->wpos = 0;
    	  	pipeptr->readerpid = -1;
			pipeptr->state = PIPE_USED;

			pipeptr->bounddev = -1;
			semreset(pipeptr->wsem, PIPE_SIZE);
			semreset(pipeptr->rsem, 0);
			pipeptr->removingwriter = 0;
			
   
    	}
    	    	
    	//this pid owns a pipe
    	if(pipeptr->ownerpid == pid){
    		//kprintf("kill pipe: %d<><><owner><><%d\n",pipeptr->pipid,pid);
			pipeptr->writerpid = -1;
    		pipeptr->wpos = 0;
    	  	pipeptr->readerpid = -1;
			pipeptr->state = PIPE_FREE;
			pipeptr->ownerpid = -1;
			pipeptr->bounddev = -1;
			semreset(pipeptr->wsem, PIPE_SIZE);
			semreset(pipeptr->rsem, 0);
			pipeptr->removingwriter = 0;
    		/**/
    	}
    }
	
	/*789*/


	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */
	

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
