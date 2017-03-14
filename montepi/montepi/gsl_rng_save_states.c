//
// typical usage: ./gsl_rng_save_states.exe -i 20000000000 -s 16
//   Saves 16 taus RNG states separated by 20000000000 samples
//   Default state save file is taus_rng_%llu_states_with_stride_%llu.dat
//   which, in this example, is taus_rng_16_states_with_stride_20000000000.dat
//
// Brad Noble - Sat Mar  4 07:29:00 CST 2017
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <gsl/gsl_rng.h>

//
// Program usage message and getopt(3) options
//
static char *usage = "[-f outfile] [-i interval] [-s states to save]\n\
                     -f <arg>, filename to save output\n\
                     -i <arg>, output interval for seed values\n\
                     -s <arg>, state values to save\n\
                     -h, this help message";
static char *options = "f:i:s:th";

//
// Globals for getopt() command line processing
//
int Fflag = 0, Iflag = 0, Sflag = 0, Tflag = 0, Hflag = 0, Errflag = 0;
char Outfile[256];
unsigned long long Interval = 0;
unsigned long long StatesToSave = 0;

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
		case 'f':
			strncpy(Outfile, optarg, 255);
			Fflag++;
			break;
		case 'i':
			Interval = strtoull(optarg, NULL, 10);
			if (Interval < 1)
			{
				printf("invalid interval = %llu\n", Interval);
				Errflag++;
			}
			Iflag++;
			break;
		case 's':
			StatesToSave = strtoull(optarg, NULL, 10);
			if (StatesToSave < 1)
			{
				printf("invalid seeds to save = %llu\n", StatesToSave);
				Errflag++;
			}
			Sflag++;
			break;
		case 't':
			Tflag++;
			break;
		case 'h':
			Hflag++;
			break;
		default:
			Errflag++;
			break;
		}
	}
	/* ----| Check for manditory arguments and errors ----| */
	if (((!(Iflag) || !(Sflag)) && (!(Tflag))) || Errflag || Hflag)
	{
		printf("usage : %s %s\n", argv[0], usage);
		exit(1);
	}
}

//
// Start the show
//
int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned long long s, i, n, cnt;
	char suffix = ' ';
	const gsl_rng_type **t, **t0;
	const gsl_rng *rng;
	double x;

	ProcessCommandLine(argc, argv);

	//
	// print rng types
	//
	if (Tflag) {
		t0 = gsl_rng_types_setup();
		for(t=t0; *t!=0; t++) {
			printf("%s\n",(*t)->name);
		}
		exit(0);
	}

	//
	// Send output to either a file or stdout
	//
	if (!Fflag)
	{
		sprintf(Outfile, "taus_rng_%llu_states_with_stride_%llu.dat", StatesToSave, Interval);
	}
	if ((fp = fopen(Outfile, "w")) == NULL)
	{
		printf("Cannot open %s: %s\n", Outfile, strerror(errno));
		exit(2);
	}

	//
	// Generate random numbers, writing the seed value every specified interval
	//
	rng = gsl_rng_alloc(gsl_rng_taus);
	printf("state_size=%u\n",(unsigned int)gsl_rng_size(rng));

	cnt = 0; n = 0; s = 0;
	while(s < StatesToSave) {
		if (gsl_rng_fwrite(fp,rng) != 0) {
			printf("gsl_rng_fwrite() failed\n");
		}
		fflush(fp);
		x = gsl_rng_uniform(rng);
		i=1;
		cnt++;
		printf("%4llu%c %.5f\n", n, suffix, x);
		while(i < Interval)
		{
			x = gsl_rng_get(rng);
			i++;
			cnt++;
		}
		if      (cnt >= 1000000000000000000) { suffix='E';  n=cnt/1000000000000000000; }
		else if (cnt >= 1000000000000000)    { suffix='P';  n=cnt/1000000000000000; }
		else if (cnt >= 1000000000000)       { suffix='T';  n=cnt/1000000000000; }
		else if (cnt >= 1000000000)          { suffix='G';  n=cnt/1000000000; }
		else if (cnt >= 1000000)             { suffix='M';  n=cnt/1000000; }
		else if (cnt >= 1000)                { suffix='k';  n=cnt/1000; }
		else                                 { suffix=' '; n=cnt; }
		s++;
	}
	cnt++;
	x = gsl_rng_uniform(rng);
	printf("%4llu%c %.5f\n", n, suffix, x);
	if (gsl_rng_fwrite(fp,rng) != 0) {
		printf("gsl_rng_fwrite() failed\n");
	}
	fflush(fp);

	//
	// Close the seed save file and exit
	//
	if (Fflag)
	{
		fclose(fp);
	}
	exit(0);
}
