/* ----------------------------------------------------------- */
/* NAME : John Mortimore                     User ID: jgmortim */
/* DUE DATE : 04/19/2019                                       */
/* PROGRAM ASSIGNMENT #5                                       */
/* FILE NAME : thread-main.cpp                                 */
/* PROGRAM PURPOSE :                                           */
/*    Santa, elves, and reindeer with monitors. My solution is */
/*    probably not elegant, but it was tested with 650,000     */
/*    runs with random integers between 1 & 25 for r, e, and t.*/
/*    No deadlocks.                                            */
/* ----------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "thread.h"

using namespace std;

/* ----------------------------------------------------------- */
/* FUNCTION : main                                             */
/*    The main function for the program.                       */
/* PARAMETER USAGE :                                           */
/*    argc {int} - number of argument.                         */
/*    argv {char**} - list of arguments.                       */
/*       argv[1] - number of elves.                            */
/*       argv[2] - number of reindeer.                         */
/*       argv[3] - number of deliveries.                       */
/* FUNCTIONS CALLED :                                          */
/*    atoi(), ElfThread::Begin(), ElfThread::Join(),           */
/*    ReindeerThread::Begin(), ReindeerThread::Join(),         */
/*    SantaThread::Begin(), SantaThread::Join(), sprintf(),    */
/*    strlen(), write()                                        */
/* ----------------------------------------------------------- */
int main(int argc, char *argv[]) {
	int e, r, t;
	int i;
	char output[1024];
	if (argc != 4) return 1;
	if ((e = atoi(argv[1])) == 0) e=7; /* Default 7 elves. */
	if ((r = atoi(argv[2])) == 0) r=9; /* Default 9 reindeer. */
	if ((t = atoi(argv[3])) == 0) t=5; /* Default 5 trips. */

	/* Create threads. */
	Santa* santa = new Santa(r, t);
	Reindeer* reindeer[r];
	Elf* elves[e];
	for (i=0; i < r; i++) reindeer[i] = new Reindeer(i+1);
	for (i=0; i < e; i++) elves[i] = new Elf(i+1);
	/* Start threads. */
	santa->Begin();
	for (i=0; i < r; i++) reindeer[i]->Begin();
	for (i=0; i < e; i++) elves[i]->Begin();
	/* Wait for threads to finish. */
	santa->Join();
	for (i=0; i < r; i++) reindeer[i]->Join();
	for (i=0; i < e; i++) elves[i]->Join();
	/* Print last message and exit. */
	sprintf(output, "After (%d) deliveries, Santa retires and is on vacation!\n", t);
	write(1, output, strlen(output));
	return 0;
}

