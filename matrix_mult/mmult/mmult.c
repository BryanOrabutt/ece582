/*
 * Matrix multiply-and-add template: C += A * B
 *
 * Creates three matrices, A, B, & C, and fills them with either random
 * floating-point values from 0.0 to 0.999... and the computes the product
 * of A and B and then adds the result to C.
 *
 * bnoble - Thu, Jan 26, 2017 12:05:11 PM
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h> /* needed for QueryPerformanceFrequency() and QueryPerformanceFrequency() */

#define _64bit (sizeof(void*) == 8)

/*
 * C-preprocessor macros
 *   MIN/MAX -- See http://stackoverflow.com/questions/3437404/min-and-max-in-c
 */
#define MIN(a,b) (((a)<(b))?(a):(b))

/*
 * Globals (not including getopt globals)
 *   These will have to be global when we make this program threaded.
 */
double **A, **B, **C;
long TimeCountStart;
double Freq;
double ElapsedTimeInSeconds;

/*
 * getopt globals
 *   These should all be initialized to something in order to check for errors
 *
 */
int N = 0;
int simple = 0;
int block = 0;
int istride = 0;
int jstride = 0;
int kstride = 0;
int timing = 0;
int debug = 0;
int out = 0;
int unity = 0;
int unknown = 0;

/*
 * getopt command-line options
 *   Options with a colon (:) require an argument.
 *
 * -s, execute the simple-sequential algorithm
 * -b, execute the block-sequential algorithm
 * -N <arg>, matrix size (NxN)
 * -i <arg>, where arg is i loop stride (default MIN(256/sizeof(double), N))
 * -j <arg>, where arg is j loop stride (default MIN(256/sizeof(double), N))
 * -k <arg>, where arg is k loop stride (default MIN(256/sizeof(double), N))
 * -t, print timing information
 * -d, turn on debug/diagnostic messages flag
 * -o, output the matrix values
 * -u, initialize the matrices with 1.0 (unity)
 *
 */
static char *options = "sbN:i:j:k:tdou";

/*
 * parse the command-line arguments and check and report any errors
 */
void parseargs(int argc, char *argv[])
{
	int c;
	int badopt = 0;
	char *progname = argv[0];

	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 's': /* execute the simple-sequential algorithm */
			simple++;
			break;
		case 'b': /* execute the block-sequential algorithm */
			block++;
			break;
		case 'N': /* matrix size (NxN) */
			if ((N = atoi(optarg)) <= 0) {
				badopt++;
			}
			break;
		case 'i': /* determine the i-loop stride to use */
			if ((istride = atoi(optarg)) <= 0)
			{
				istride = MIN(256/sizeof(double), N);
			}
			break;
		case 'j': /* determine the j-loop stride to use */
			if ((jstride = atoi(optarg)) <= 0)
			{
				jstride = MIN(256/sizeof(double), N);
			}
			break;
		case 'k': /* determine the k-loop stride to use */
			if ((kstride = atoi(optarg)) <= 0)
			{
				kstride = MIN(256/sizeof(double), N);
			}
			break;
		case 't': /* print timing information */
			timing++;
			break;
		case 'd': /* print debug/diagnostic messages */
			debug++;
			break;
		case 'o': /* output the initial and final matrix values */
			out++;
			break;
		case 'u': /* unity matrices instead of random doubles */
			unity++;
			break;
		default:
			unknown++;
			badopt++;
		}
	}
	/* simple or block sequential algorithm must be specified */
	if (((simple == 0)&&(block == 0))||((simple)&&(block))) {
		printf("Must specify either -s (simple) or -b (block) sequential algorithm.\n");
		badopt++;
	}
	/* sanity check on N */
	if (N == 0) {
		printf("N is required and must be greater than 0.\n");
		badopt++;
	}
	/* sanity check on {i,j,k}stride values */
	if ((simple)&&((istride)||(jstride)||(kstride))) {
		printf("{i,j,k} block sizes are not used in the simple sequential algorithm.\n");
		badopt++;
	}
	/* if block sequential algorithm is used set the {i,j,k}stride values */
	if (block) {
		if ((!istride)||(!jstride)||(!kstride)) {
			printf("note: using default block sizes:");
			if (istride == 0)	{
				istride = MIN(256/sizeof(double), N);
				printf(" istride=%d",istride);
			}
			if (jstride == 0)	{
				jstride = MIN(256/sizeof(double), N);
				printf(" jstride=%d",jstride);
			}
			if (kstride == 0)	{
				kstride = MIN(256/sizeof(double), N);
				printf(" kstride=%d",kstride);
			}
			printf("\n");
		}
	}
	/* notify of any unknown command-line options */
	if (unknown) {
		printf("unknown command-line option\n");
	}
	/* print a usage message for any bad command-line */
	if (badopt || optind < argc) {
		fprintf(stderr,
		        "usage: %s -N size -b|-k [-i istride] [-j jstride] [-k kstride] [-t] [-o] [-d] [-u]\n",
		        progname);
		exit(0);
	}
}

/*
 * Initialize timing parameters
 * Based on: https://cygwin.com/ml/cygwin/2000-03/msg00579.html
 */
void initialize_time(void)
{
	LARGE_INTEGER lFreq, lCnt;

	QueryPerformanceFrequency(&lFreq);
	Freq = (_64bit) ? (double)lFreq.QuadPart:(double)lFreq.LowPart;
	QueryPerformanceCounter(&lCnt);
	TimeCountStart = (_64bit) ? lCnt.QuadPart:lCnt.LowPart;
}

void elapsed_time(void)
{
	LARGE_INTEGER lCnt;
	long tcnt;

	QueryPerformanceCounter(&lCnt);
	tcnt = (_64bit) ? (lCnt.QuadPart - TimeCountStart):(lCnt.LowPart - TimeCountStart);
	ElapsedTimeInSeconds = ((double)tcnt)/Freq;
}

/*
 * initialize matrices A, B, & C
 */
void initialize(void)
{
	int i, j;
	double *p1;
	double *p2;
	double *p3;

	A = (double **) malloc(N*sizeof(double *));
	B = (double **) malloc(N*sizeof(double *));
	C = (double **) malloc(N*sizeof(double *));

	p1 = (double *) memalign(getpagesize(),N*N*sizeof(double));
	p2 = (double *) memalign(getpagesize(),N*N*sizeof(double));
	p3 = (double *) memalign(getpagesize(),N*N*sizeof(double));

	for (i = 0 ; i < N; i++, p1 += N, p2 += N, p3 += N) {
		A[i] = p1;
		B[i] = p2;
		C[i] = p3;
	}

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			A[i][j] = ((debug || unity) ? 1.0 : drand48());
			B[i][j] = ((debug || unity) ? 1.0 : drand48());
			C[i][j] = ((debug || unity) ? 1.0 : drand48());
		}
	}
}

/*
 * compute C += A * B using a simple cache oblivious algorithm
 */
void simple_sequential(void)
{
	register int i, j, k;
	double sum;

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			sum = 0.0;
			for (k = 0; k < N; k++) {
				sum += A[i][k] * B[k][j];
			}
			C[i][j] += sum;
		}
	}
}

/*
 * compute C += A * B using a simple cache aware block algorithm
 */
void block_sequential(void)
{
	register int i, j, k, kk, ii, jj;
	double sum;
	int I, J, K;

	if (debug) printf("istride=%d, jstride=%d, kstride=%d\n",istride,jstride,kstride);
	for (ii = 0; ii < N; ii += istride) {
		for (jj = 0; jj < N; jj += jstride) {
			for (kk = 0; kk < N; kk += kstride) {
				I = MIN(ii+istride,N);
				for (i = ii; i < I; i++) {
					J = MIN(jj+jstride,N);
					for (j = jj; j < J; j++) {
						K = MIN(kk+kstride,N);
						sum = 0.0;
						for (k = kk; k < K; k++) {
							sum += A[i][k] * B[k][j];
						}
						C[i][j] += sum;
					}
				}
			}
		}
	}
}

void printarray(double **A)
{
	int i, j;

	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) printf(" %0.1e", A[i][j]);
		putchar('\n');
	}
}

int main(int argc, char *argv[])
{
	parseargs(argc, argv);
	if (debug) {
		printf("System page size is %d\n",getpagesize());
	}
	initialize();
	if (out) {
		printf("A =\n");
		printarray(A);
		printf("B =\n");
		printarray(B);
		printf("C =\n");
		printarray(C);
	}
	if (simple) {
		initialize_time();
		simple_sequential();
		elapsed_time();
		if (timing) printf("%f\n",ElapsedTimeInSeconds);
	}
	else if (block) {
		initialize_time();
		block_sequential();
		elapsed_time();
		if (timing) printf("%f\n",ElapsedTimeInSeconds);
	}
	if (out) {
		printf("C =\n");
		printarray(C);
	}
	return(0);
}
