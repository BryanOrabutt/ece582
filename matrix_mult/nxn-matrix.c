#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char** argv)
{
    //initial variable declarations
    int n, t, ch, row, col, m, max;
    n = 0;
    max = 0;

    //create file handles and open files to store data points
    FILE* fid0, *fid1, *fid2, *fid3;
    fid0 = (FILE*)fopen("c.txt", "w");
    fid1 = (FILE*)fopen("b.txt", "w");
    fid2 = (FILE*)fopen("a.txt", "w");
    fid3 = (FILE*)fopen("result.txt", "w");

    //seed the random generator and set the option error variable
    srand(time(NULL));
    opterr = 1;

    //check for command line inputs.
    while((ch = getopt(argc, argv, "m:n:t:")) != -1)
    {
        switch(ch)
        {
            case 'm':
                max = atoi(optarg);
                break;
            case 'n':
                n = atoi(optarg);
                break;
            case 't':
                fprintf(stderr, "Threading has not yet been implemented.\n", optopt);
                return 1;
            default:
                abort();
        }
    }

    if(!n) n = 4; //if n was not given use 4
    if(!max) max = 10; //if max not specified use 10.

    //create matrices
    int a[n][n];
    int b[n][n];
    int c[n][n];

    //fill matrices.
    for(row = 0; row < n; row++)
    {
        for(col = 0; col < n; col++)
        {
            a[row][col] = rand()%max;
            b[row][col] = rand()%max;
            c[row][col] = rand()%max;
            fprintf(fid0, "%d\t", c[row][col]);
        }
        fprintf(fid0, "\n");
    }

    //compute c = c + a*b
    for(row = 0; row < n; row++)
    {
        for(col = 0; col < n; col++)
        {
            for(m = 0; m < n; m++)
            {
                c[row][col] += a[row][m]*b[m][col];
            }
        }
    }

    //write results to a file.
    for(row = 0; row < n; row++)
    {
        for(col = 0; col < n; col++)
        {
            fprintf(fid3, "%d\t", c[row][col]);
            fprintf(fid1, "%d\t", b[row][col]);
            fprintf(fid2, "%d\t", a[row][col]);
        }
        fprintf(fid3, "\n");
        fprintf(fid1, "\n");
        fprintf(fid2, "\n");
    }

    //close files.
    fclose(fid0);
    fclose(fid1);
    fclose(fid2);
    fclose(fid3);
    return 0;
}
