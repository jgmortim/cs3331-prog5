/* ----------------------------------------------------------- */
/* NAME : John Mortimore                     User ID: jgmortim */
/* DUE DATE : 04/19/2019                                       */
/* PROGRAM ASSIGNMENT #5                                       */
/* FILE NAME : thread.cpp                                      */
/* PROGRAM PURPOSE :                                           */
/*    Contains all class implementations for my threads.       */
/* ----------------------------------------------------------- */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "thread.h"

using namespace std;
JMMonitor *mon;

/* ----------------------------------------------------------- */
/* FUNCTION : constructor                                      */
/*    Creates a new JMMonitor. See thread.h for details of     */
/*    variables and conditions.                                */
/* PARAMETER USAGE :                                           */
/*    r {int} - number of reindeer.                            */
/*    t {int} - number of deliveries to make.                  */
/* FUNCTIONS CALLED :                                          */
/*    Condition Constructor                                    */
/* ----------------------------------------------------------- */
JMMonitor::JMMonitor(int r, int t):Monitor("monitor", HOARE){
	isAsleep=false;
	isSleighReady=false;
	isTripComplete=false;
	isQuestionAnswered=false;
	isGroupWaiting=false;
	lastDeer=false;
	ReindeerTotal = r;
	ReindeerRet = 0;
	ReindeerAvai = 0;
	ReindeerAttach = 0;
	delivMax = t;
	delivCount = 0;
	ElvesWaiting = 0;
	GroupReleased = new Condition("ElvesReleased");
	QuestionAnswered = new Condition("QuestionAnswered");
	Wake = new Condition("SantaAwake");
	AllReindeerRet = new Condition("AllDeerBack");
	AllReindeerAvai = new Condition("AlOutOfHut");
	Sleigh = new Condition("SleighReady");
	Fly = new Condition("FlyAway");
	TripComplete = new Condition("TripComplete");
	Asleep0 = new Condition("RetToElf");
	Asleep1 = new Condition("RetToReindeer");
}

/* ----------------------------------------------------------- */
/* FUNCTION : constructor                                      */
/*    Creates a new Elf thread.                                */
/* PARAMETER USAGE :                                           */
/*    id {int} - the id of the elf.                            */
/* FUNCTIONS CALLED :                                          */
/*    sprintf(), strlen(), ThreadName.seekp(), write()         */
/* ----------------------------------------------------------- */
Elf::Elf(int id) {
	char output[1024];
	sprintf(output, "          Elf %d starts\n", id);
	write(1, output, strlen(output));
	ID=id;
	ThreadName.seekp(0, ios::beg);
	ThreadName << "Elf_" << ID << '\0';
}

/* ----------------------------------------------------------- */
/* FUNCTION : constructor                                      */
/*    Creates a new Reindeer thread.                           */
/* PARAMETER USAGE :                                           */
/*    id {int} - the id of the reindeer.                       */
/* FUNCTIONS CALLED :                                          */
/*    sprintf(), strlen(), ThreadName.seekp(), write()         */
/* ----------------------------------------------------------- */
Reindeer::Reindeer(int id) {
	char output[1024];
	sprintf(output, "     Reindeer %d starts\n", id);
	write(1, output, strlen(output));
	ID=id;
	ThreadName.seekp(0, ios::beg);
	ThreadName << "Reindeer_" << ID << '\0';
}

/* ----------------------------------------------------------- */
/* FUNCTION : constructor                                      */
/*    Creates a new Santa thread.                              */
/* PARAMETER USAGE :                                           */
/*    r {int} - number of reindeer.                            */
/*    t {int} - the number of deliveries to make.              */
/* FUNCTIONS CALLED :                                          */
/*    strlen(), ThreadName.seekp(), write()                    */
/* ----------------------------------------------------------- */
Santa::Santa(int r, int t) {
	char *output = "Santa thread starts\n";
	write(1, output, strlen(output));
	mon = new JMMonitor(r, t);
	deliveryMax = t;
	deliveryCount = 0;
	reindeerTotal = r;
	ThreadName.seekp(0, ios::beg);
	ThreadName << "Santa" << '\0';
}

/* ----------------------------------------------------------- */
/* FUNCTION : Sleep                                            */
/*    Santa takes a nap.                                       */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd()                                             */
/* ----------------------------------------------------------- */
void JMMonitor::Sleep() {
	MonitorBegin();
	char *output="Santa takes a nap.\n"; // Santa wants to sleep.
	write(1, output, strlen(output));
	/* If reindeer need him, he must stay awake and tell the */
	/* last deer that he is awake.                           */
	if (ReindeerRet == ReindeerTotal) {
		isAsleep = false;
		Asleep1->Signal(); // handle deer first
	/* If the elves need him, he must stay awake and tell the */
	/* group that he is awake.                                */
	} else if (ElvesWaiting == 3) {
		isAsleep = false;
		Asleep0->Signal();
	/* Otherwise, he sleeps and waits to be woken up. */
	} else {
		isAsleep = true;
		Wake->Wait();
		isAsleep = false;
	}
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : ReindeerBack                                     */
/*    The reindeer return from vacation. The last deer wakes   */
/*    up santa. The others are free to go to the warming hut.  */
/* PARAMETER USAGE :                                           */
/*    id {int} - the id of the reindeer.                       */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd(), sprintf(),strlen(), write()                */
/* ----------------------------------------------------------- */
int JMMonitor::ReindeerBack(int id) {
	MonitorBegin();
	char output[1024];
	int ret=0;
	ReindeerRet++;
	/* The last reindeer does not go to the warming hut. The deer goes   */
	/* to find Santa. If santa is asleep, wake him up. If Santa is not   */
	/* in bed, he must be helping elves; wait for him to return and tell */
	/* him to prep the sleigh. Then call the other deer from warming hut.*/
	if (ReindeerRet == ReindeerTotal) {
		ret=1; // will not go to warming hut
		sprintf(output, "     The last reindeer %d wakes up Santa\n", id);
		if (isAsleep == true) { // Santa is sleeping.
			write(1, output, strlen(output));
			lastDeer=true;
			Wake->Signal();	
		} else { // Santa is busy with a group of elves.
			Asleep1->Wait();
			lastDeer=true;
			write(1, output, strlen(output));
		}
		AllReindeerRet->Signal(); // Call other deer.
	/* Not last deer, goto warming hut to wait. */
	} else {
		sprintf(output, "     Reindeer %d returns\n", id);
		write(1, output, strlen(output));
	}
	MonitorEnd();
	return ret; // last deer returns 1; others return 0.
}

/* ----------------------------------------------------------- */
/* FUNCTION : WaitOthers                                       */
/*    The reindeer wait in the warming hut until all reindeer  */
/*    return and santa is awake.                               */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd()                                             */
/* ----------------------------------------------------------- */
void JMMonitor::WaitOthers() {
	MonitorBegin();
	/* Deer arive at the warming hut. If the last deer has not returned  */
	/* yet, wait for the call from the last deer.                        */
	if (ReindeerRet!=ReindeerTotal) AllReindeerRet->Wait(); 
	AllReindeerRet->Signal();
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : WaitSleigh                                       */
/*    The reindeer wait as a group for the sleigh to be ready. */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd()                                             */
/* ----------------------------------------------------------- */
void JMMonitor::WaitSleigh() {
	MonitorBegin();
	ReindeerAvai++;
	/* Wait until all deer have arrived from the warming hut.*/
	if (ReindeerAvai == ReindeerTotal) { // Last deer signals all ready.
		AllReindeerAvai->Signal();
	} else {
		AllReindeerAvai->Wait();
		AllReindeerAvai->Signal();
	}
	/* If the sleigh is still not ready, wait for Santa to finish */
	/* preparing the sleigh.                                      */
	if (isSleighReady==false) { // If sleigh not ready, wait.
		Sleigh->Wait();
		Sleigh->Signal();
	} // All deer goto together.
	ReindeerRet = 0;
	lastDeer=false;
	ReindeerAvai--;
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : PrepSleigh                                       */
/*    When all of the deer have return, Santa prepares the     */
/*    sleigh. Then the reindeer can be attached.               */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), MonitorBegin(), MonitorEnd(),       */
/*    strlen(), write()                                        */
/* ----------------------------------------------------------- */
void JMMonitor::PrepSleigh() {
	MonitorBegin();
	/* Santa preps the Sleigh then tells the deer that it is ready. */
	char *text1 = "Santa is preparing sleigh\n";
	write(1, text1, strlen(text1));
	isTripComplete=false;
	isSleighReady=true;
	Sleigh->Signal();
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : StoreSleigh                                      */
/*    When a trip is finished and the deer released, put the   */
/*    sleigh away. Then Santa can go back to sleep.            */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd()                                             */
/* ----------------------------------------------------------- */
void JMMonitor::StoreSleigh() {
	MonitorBegin();
		if (isTripComplete==false) {
			TripComplete->Wait();
			TripComplete->Signal();
		}
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : FlyOff                                           */
/*    Deer are attached to the sleigh and fly off.             */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd(), sprintf(), strlen(), write()               */
/* ----------------------------------------------------------- */
void JMMonitor::FlyOff() {
	char output[1024];
	MonitorBegin();
	/* Attach each of the reindeer to the sleigh. When the last deer */
	/* is attached to the sleigh, the deer and santa fly off.        */
	ReindeerAttach++;
	if (ReindeerAttach == ReindeerTotal) {
		delivCount++;
		sprintf(output, "The team flies off (%d)!\n", delivCount);
		write(1, output, strlen(output));
		Fly->Signal();
	} else {
		Fly->Wait();
		Fly->Signal();
	}
	/* After the presents are delivered. Detach each deer from the      */
	/* sleigh. The last deer detached declares the trip complete.       */
	/* When the trip is over, santa can put the sleigh away and go      */
	/* back to sleep. The deer are free to leave when trip is complete. */
	ReindeerAttach--;
	if (ReindeerAttach == 0) { // last deer; trip over
		isSleighReady = false;
		isTripComplete = true;	// santa can put sleigh away
		TripComplete->Signal(); // and deer can go on vacation
	}
	if (isTripComplete == false) { // wait till all deer are detached
		TripComplete->Wait();
		TripComplete->Signal(); // deer can go on vacation
	}
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : AskQuestion                                      */
/*    Used by elves to form group and ask Santa questions.     */
/* PARAMETER USAGE :                                           */
/*    id {int} - the id of the elf.                            */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd(), sprintf(), strlen(), write()               */
/* ----------------------------------------------------------- */
void JMMonitor::AskQuestion(int id) {
	char output[1024];
	MonitorBegin();
	/* If there is a group of elves waiting for santa, all other elves */
	/* must wait for the group to be released.                         */
	if (isGroupWaiting && delivCount < delivMax) {
		GroupReleased->Wait();
	}
	ElvesWaiting++;
	/* Elves 1 and 2 start the group and the 3rd elf blocks all other    */
	/* elves from joining the group. If santa is asleep, the group wakes */
	/* him up. If Santa is not in bed, he must be busy with the reindeer.*/
	/* So the elves wait for him to return. After the questions have     */
	/* been answered. The last elf in the monitor releases the barriers  */
	/* preventing more elves from asking questions.                      */
	if (delivCount < delivMax) {	
		sprintf(output, "          Elf %d has a problem\n", id);
		write(1, output, strlen(output));
		if (ElvesWaiting == 3) { // Elf 3
			isGroupWaiting=true;
			group[ElvesWaiting-1] = id;
			sprintf(output, "          Elves %d, %d, %d wake up Santa\n", group[0], group[1], group[2]);
			if (isAsleep == true) { // if santa is sleeping	
				write(1, output, strlen(output));
				Wake->Signal();	// wake him up
			} else {	//else, wait for santa to return
				Asleep0->Wait();
				if (delivCount < delivMax) write(1, output, strlen(output));
			}
		} else { // elf 1 or 2
			isQuestionAnswered=false;
			group[ElvesWaiting-1] = id;	
			GroupReleased->Signal();
		}
		/* Wait for question to be answered. */
		if (isQuestionAnswered==false && delivCount < delivMax) QuestionAnswered->Wait();
		ElvesWaiting--;
		if (ElvesWaiting == 0) { // Last elf releases the group.
			isGroupWaiting=false;
			GroupReleased->Signal();
		}
	}
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : AnswerQuestion                                   */
/*    Used by Santa to answer the questions of a group of      */ 
/*    elves.                                                   */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), MonitorBegin(), MonitorEnd(),       */
/*    sprintf(), strlen(), write()                             */
/* ----------------------------------------------------------- */
void JMMonitor::AnswerQuestion() {
	char output[1024];
	MonitorBegin();
	/* Santa answers all three questions of the elf group. */
	sprintf(output, "Santa answers the question posted by elves %d, %d, %d\n", group[0], group[1], group[2]);
	write(1, output, strlen(output));
	isQuestionAnswered = true;
	QuestionAnswered->Signal();
	QuestionAnswered->Signal();
	QuestionAnswered->Signal();
	MonitorEnd();
}

/* ----------------------------------------------------------- */
/* FUNCTION : ReindeerRetire                                   */
/*    Used by reindeer to check if they should retire.         */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    MonitorBegin(), MonitorEnd()                             */
/* ----------------------------------------------------------- */
int JMMonitor::ReindeerRetire() {
	int ret=0;
	MonitorBegin();
	if (delivCount == delivMax) ret = 1;
	MonitorEnd();
	return ret;
}

/* ----------------------------------------------------------- */
/* FUNCTION : ElfRetire                                        */
/*    Used by elves to check if they should retire. Also       */
/*    signals any elves that may be wait so they can exit.     */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Condition::Signal(), Condition::Wait(), MonitorBegin(),  */
/*    MonitorEnd()                                             */
/* ----------------------------------------------------------- */
int JMMonitor::ElfRetire() {
	int ret=0;
	MonitorBegin();
	if (delivCount == delivMax) {
		ret = 1;
		GroupReleased->Signal();
		QuestionAnswered->Signal();
		QuestionAnswered->Signal();
		QuestionAnswered->Signal();
		Asleep0->Signal();
	}
	MonitorEnd();
	return ret;
}

/* ----------------------------------------------------------- */
/* FUNCTION : DeerCheck                                        */
/*    Used by Santa to confirm that all deer have returned.    */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    MonitorBegin(), MonitorEnd()                             */
/* ----------------------------------------------------------- */
bool JMMonitor::DeerCheck() {
	MonitorBegin();
	bool ret=lastDeer;
	MonitorEnd();
	return ret;
}

/* ----------------------------------------------------------- */
/* FUNCTION : ThreadFunc                                       */
/*    Process for the elf threads to perform when begin() is   */
/*    called.                                                  */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Delay(), JMMonitor::AskQuestion(),                       */
/*    JMMonitor::ElfRetire(), Thread::ThreadFunc(),            */
/* ----------------------------------------------------------- */
void Elf::ThreadFunc() {
	Thread::ThreadFunc();
	while (1) {
		Delay(1000);		// make toys
		mon->AskQuestion(ID); // encounter a problem
		Delay(100); 		// problem solved, take a rest
		if (mon->ElfRetire()==1) Exit(); // retire if necessary 
	}
}

/* ----------------------------------------------------------- */
/* FUNCTION : ThreadFunc                                       */
/*    Process for the reindeer threads to perform when begin() */
/*    is called.                                               */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Delay(), Exit(), JMMonitor::FlyOff(),                    */
/*    JMMonitor::ReindeerBack(), JMMonitor::ReindeerRetire(),  */
/*    JMMonitor::WaitOthers(), JMMonitor::WaitSleigh(),        */
/*    Thread::ThreadFunc()                                     */
/* ----------------------------------------------------------- */
void Reindeer::ThreadFunc() {
	Thread::ThreadFunc();
	int x;
	while (1) {
		Delay(1000); // tan on the beach
		x=mon->ReindeerBack(ID); // report back to santa
		if (x==0) mon->WaitOthers(); // wait for others, expt last deer
		mon->WaitSleigh(); // wait for sleigh
		mon->FlyOff(); //fly off to deliver toys
		Delay(10); // prepare for vacation
		if (mon->ReindeerRetire()==1) Exit(); // retire if necessary 
	}
}

/* ----------------------------------------------------------- */
/* FUNCTION : ThreadFunc                                       */
/*    Process for the santa thread to perform when begin() is  */
/*    called.                                                  */
/* PARAMETER USAGE :                                           */
/*    N/A                                                      */
/* FUNCTIONS CALLED :                                          */
/*    Delay(), Exit(), JMMonitor::AnswerQuestion(),            */
/*    JMMonitor::DeerCheck(), JMMonitor::ElfRetire(),          */
/*    JMMonitor::PrepSleigh(), JMMonitor::Sleep(),             */
/*    JMMonitor::StoreSleigh(), Thread::ThreadFunc()           */
/* ----------------------------------------------------------- */
void Santa::ThreadFunc() {
	Thread::ThreadFunc();
	while (deliveryCount < deliveryMax) {
		mon->Sleep(); // Take a nap.
		if (mon->DeerCheck()) { // Deliver presents.
			mon->PrepSleigh();
			deliveryCount++;
			Delay(500); 
			mon->StoreSleigh();
     		} else { // help elves.
			Delay(100);
			mon->AnswerQuestion();
		}
	}
	mon->ElfRetire(); // Unblock elves; needed for when all are blocked.
	Exit(); // retire.
}
