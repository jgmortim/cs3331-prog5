/* ----------------------------------------------------------- */
/* NAME : John Mortimore                     User ID: jgmortim */
/* DUE DATE : 04/19/2019                                       */
/* PROGRAM ASSIGNMENT #5                                       */
/* FILE NAME : thread.h                                        */
/* PROGRAM PURPOSE :                                           */
/*    Contains all class definitions for my threads.           */
/* ----------------------------------------------------------- */

#include "ThreadClass.h"

class JMMonitor : public Monitor {
	public:
		JMMonitor(int r, int t);
		void AskQuestion(int id);
		int ReindeerBack(int id);
		void WaitOthers();
		void WaitSleigh();
		void FlyOff();
		void Sleep();
		void PrepSleigh();
		void StoreSleigh();
		void AnswerQuestion();
		int ReindeerRetire();
		int ElfRetire();
		bool DeerCheck();
	private:
		bool isAsleep;		// See *Wake
		bool isSleighReady;	// See *Sleigh
		bool isQuestionAnswered;// See *QuestionAnswered
		bool isGroupWaiting;	// blocks new elves
		bool isTripComplete;	// See *TripComplete
		bool lastDeer; // if last deer is waiting for santa
		int group[3]; // the id's of the waiting elves
		int ElvesWaiting;	// The number of elves waiting
		int ReindeerTotal;	// total num of deer
		int delivCount;		// num of completed deliveries
		int delivMax;		// max num of deliveries
		int ReindeerRet;	// num deer returned
		int ReindeerAvai;	// num deer ready for sleigh
		int ReindeerAttach;	// num deer attached to sleigh
		Condition *Asleep0;	// Can't sleep b/c elves
		Condition *Asleep1;	// Can't sleep b/c reindeer
		Condition *Wake;	// wake up santa
		Condition *AllReindeerRet; // All reindeer have returned
		Condition *AllReindeerAvai;// All reindeer are ready to leave
		Condition *Sleigh;	// The sleigh is ready
		Condition *Fly;		// The deer fly off with the sleigh
		Condition *TripComplete; // A delivery has been completed
		Condition *GroupReleased; // Elf group dismissed
		Condition *QuestionAnswered; // Questions have been answered
};

class Elf : public Thread {
	public:
		Elf(int id);
	private:
		int ID;
		void ThreadFunc();
};


class Reindeer : public Thread {
	public:
		Reindeer(int id);
	private:
		int ID;
		void ThreadFunc();
};

class Santa : public Thread {
	public:
		Santa(int r, int t);
	private:
		int reindeerTotal;
		int deliveryCount;
		int deliveryMax;
		void ThreadFunc();
};
