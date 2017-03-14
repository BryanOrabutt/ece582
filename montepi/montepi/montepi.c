/* --------------------------------------------------------------------------
 * ----| montepi.c
 * ----|
 * ----| Computes pi using Monte Carlo simulation.
 * ----| See "Numerical Recipes in C" for details on error estimation.
 * --------------------------------------------------------------------------
 * ----| Formatted with: astyle -p --style=ansi monte_pi.c
 * ----| Printed with: enscript -GEc -fCourier7 -Pmyprinter monte_pi.c
 * ----| HTML Format with:  enscript -Whtml -Ec monte_pi.c -o monte_pi.c.html
 * --------------------------------------------------------------------------
 * ----| Brad Noble - Tue, Feb 21, 2017  9:04:55 AM
 * --------------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <gsl/gsl_rng.h>

//
// Defined compilation switches
//
//#define BOUND_THREADS	// uncomment to make threads bound at creation (PTHREAD_SCOPE_SYSTEM)

//
// Constants and defaults
//
#define	DEFAULT_NUMBER_OF_THREADS 1
#define	DEFAULT_THROWS 100000000
#define MAXTHREADS 64

//
// Structure for passing arguments to threads
//
struct thread_arg
{
	int id;
	unsigned long long throws;
	unsigned long long hits;
	gsl_rng *rng;
};

//
// Structure for thread locks and conditions
//
struct
{
	pthread_mutex_t lock;
	pthread_cond_t done;
	int count;
} Work;

//
// Program usage message and getopt(3) options
//
static char *usage = "[-d] [-f Outfilefile] [-h] [-s] [-t throws] [-i iterations]\n \
                 -d, turn on debugging messages\n \
                 -f <arg>, where arg is a file name for Outfile\n \
                 -h, print this help message and exit\n \
                 -p <arg>, number of pthreads\n \
                 -r <arg>, file containing GSL RNG states\n \
                 -t <arg>, number of throws per iteration\n \
                 -s, print wall-clock timing summary";
static char *options = "df:ht:p:r:s";

//
// C pre-processor Macros
//
#define SQUARE(a) ((a)*(a))

//
// Globals for getopt() command line processing
//
int Dflag = 0, Fflag = 0, Hflag = 0, Rflag = 0, Tflag = 0, Sflag = 0, Errflag = 0;
char Outfile[256];
char RNGStateFile[256];
unsigned long long TotalThrows = DEFAULT_THROWS;
unsigned Nthreads = DEFAULT_NUMBER_OF_THREADS;

//
// Use getopt(3) to process command line arguments
//
void ProcessCommandLine(int argc, char *argv[])
{
	char ch;

	/* ----| Parse arguments.  Flags that are followed by
	 * ----| a colon require an argument, e.g. -f Outfile
	 */
	while ((ch = getopt(argc, argv, options)) != EOF)
	{
		switch (ch)
		{
		case 'd':
			Dflag++;
			break;
		case 'h':
			Hflag++;
			break;
		case 'p':
			Nthreads = atoi(optarg);
			if (Nthreads < 1)
			{
				printf("invalid threads = %d\n", Nthreads);
				Errflag++;
			}
			break;
		case 'f':
			strncpy(Outfile, optarg, 255);
			Fflag++;
			break;
		case 'r':
			strncpy(RNGStateFile, optarg, 255);
			Rflag++;
			break;
		case 's':
			Sflag++;
			break;
		case 't':
			TotalThrows = atoi(optarg);
			if (TotalThrows < 1)
			{
				printf("invalid TotalThrows = %llu\n", TotalThrows);
				Errflag++;
			}
			Tflag++;
			break;
		default:
			Errflag++;
			break;
		}
	}
	/* ----| Check for manditory arguments and errors ----| */
	if (!Rflag || Hflag || Errflag)
	{
		printf("usage : %s %s\n", argv[0], usage);
		exit(1);
	}
}

//
// Returns a long integer scaled to GetMilliseconds
//
long GetMilliseconds(void)
{
	static struct timeval tp;

	gettimeofday(&tp, (struct timezone *)0);
	return (tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

//
// Throws argument "throws" darts at a unit square (0.0-1.0, 0.0-1.0) and
// returns the total number of that hit within a quarter circle of unit radius.
//
void *ThrowDarts(void *thrarg)
{
	struct thread_arg *myarg;
	int i;
	double x,y;
	gsl_rng *myrng;
	unsigned long long myhits = 0; // our local hit count 

	//
	// Extract the arguments passed to us
	//
	myarg = (struct thread_arg *)thrarg;

	//
	// Make a local copy of the RNG given to us. Not having a local copy
	// of this or the hits variable is the reason I was having trouble
	// getting speedup. Can you figure out why? - bnoble
	//
	myrng = gsl_rng_alloc(gsl_rng_taus);
	gsl_rng_memcpy(myrng, myarg->rng);

	//
	// Throw them darts
	//
	for (i = 0 ; i < myarg->throws ; i++)
	{
		x = gsl_rng_uniform(myrng);
		y = gsl_rng_uniform(myrng);
		if (SQUARE(x) + SQUARE(y) <= 1.0)
		{
			myhits++;
		}
	}
	//
	// Transfer our hit count back to main()'s variable
	//
	myarg->hits = myhits;

	//
	// Signal that we're done and exit the thread
	//
	pthread_mutex_lock(&Work.lock);
	Work.count--;
	if (Work.count == 0)
		pthread_cond_signal(&Work.done);
	pthread_mutex_unlock(&Work.lock);
	pthread_exit(NULL);
}

//
// Start the show
//
int main(int argc, char *argv[])
{
	FILE *rngfp;
	FILE *outfp;
	pthread_t *threads;
#ifdef BOUND_THREADS
	pthread_attr_t tattr;
#endif
	struct thread_arg *thrarg;
	double estpi = 0;
	unsigned long long sumhits = 0;
	long stime, etime;
	int i;
	time_t tloc;

	ProcessCommandLine(argc, argv);

	//
	// Send output to either a file or stdout
	//
	if (Fflag)
	{
		if ((outfp = fopen(Outfile, "w")) == NULL)
		{
			printf("Cannot open %s: %s\n", Outfile, strerror(errno));
			exit(2);
		}
	}
	else
	{
		outfp = stdout;
	}

	//
	// Initalize the mutex locks
	//
	pthread_mutex_init(&Work.lock, NULL);
	pthread_cond_init(&Work.done, NULL);

	//
	// Initialize the other stuff
	//
	Work.count = Nthreads;
	threads = (pthread_t *)malloc(Nthreads * sizeof(pthread_t));
	thrarg = (struct thread_arg *)malloc(Nthreads * sizeof(struct thread_arg));

	//
	// Print wall-clock summary if requested
	//
	if (Sflag)
	{
		time(&tloc);
		fprintf(outfp, "Started %s", ctime(&tloc));
	}

	//
	// Open the GSL RNG state save file
	//
	if ((rngfp = fopen(RNGStateFile, "r")) == NULL)
	{
		printf("Cannot open %s for reading: %s\n", RNGStateFile, strerror(errno));
		exit(3);
	}
	//
	// Initialize the thread arguments
	//
	for (i = 0; i < Nthreads; i++) {
		thrarg[i].id = i;
		thrarg[i].throws = TotalThrows/Nthreads;
		thrarg[i].hits = 0;
		thrarg[i].rng = gsl_rng_alloc(gsl_rng_taus);
		if (gsl_rng_fread(rngfp,thrarg[i].rng) != 0) {
			printf("thrarg[%d]: gsl_rng_fread() failed\n",i);
			exit(4);
		}
	}
	//
	// If throws-per-thread doesn't divide evenly, give the extra to thread 0
	//
	thrarg[0].throws += TotalThrows - Nthreads*thrarg[0].throws;

	//
	// Close the GSL RNG state save file
	//
	fclose(rngfp);

	//
	// Begin timing - Always compute the starting and ending time.
	// Don't test to see if the user wants it so that we don't include the
	// overhead of checking in the run-time.
	//
	stime = GetMilliseconds();

	//
	// Spawn and unleash the threads!
	//
#ifdef BOUND_THREADS
	pthread_attr_init(&tattr);
	pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
#endif
	for (i = 0; i < Nthreads; i++)
	{
#ifdef BOUND_THREADS
		pthread_create(&threads[i], &tattr, ThrowDarts, &thrarg[i]);
#else
		pthread_create(&threads[i], NULL, ThrowDarts, &thrarg[i]);
#endif
	}

	//
	// main() waits until all of the threads are done
	//
	// How this works:
	//   1. Acquire the mutex lock
	//   2. pthread_cond_wait releases the mutex lock and blocks main() until
 	//      the condition variable Work.done is true. When condition Work.done
 	//      is true, main() resumes having reacquired the mutex lock
	//   3. If Work.count is 0, all threads are done so main() releases Work.lock
	//
	// This approach is called Barrier Synchronization
	//
	pthread_mutex_lock(&Work.lock);
	while (Work.count > 0)
		pthread_cond_wait(&Work.done, &Work.lock);
	pthread_mutex_unlock(&Work.lock);

	//
	// End timing - See the note above about overhead
	//
	etime = GetMilliseconds();
	fprintf(outfp, "%ld milliseconds\n", etime - stime);

	//
	// Compute our estimate of pi
	//
	if (Dflag) {
		sumhits = 0;
		for (i = 0; i < Nthreads; i++)
			sumhits += thrarg[i].hits;
		estpi = 4.0 * (double)sumhits / (double)TotalThrows;
		fprintf(outfp, "%llu/%-llu = %.15f : error %.15f\n", sumhits, TotalThrows, estpi, M_PI-estpi);
	}

	//
	// If the output was saved to a file, close it now.
	//
	if (Fflag)
	{
		fclose(outfp);
	}

	//
	// Print wall-clock summary if requested
	//
	if (Sflag)
	{
		time(&tloc);
		fprintf(outfp, "Completed %s", ctime(&tloc));
	}
	exit(0);
}
