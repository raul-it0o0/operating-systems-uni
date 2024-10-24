#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "functions.h"
#define FIRST_N_PRIMES 100

int main (int argc, char **argv)
{
    int aflag = 0;
    int bflag = 0;
    char* cvalue = NULL;
    int dflag = 0;
    char* evalue = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (argc, argv, "abc:de:")) != -1)
        // options available: a, b and c
        // : next to c means option c requires an argument
        switch (c)
        {
            case 'a':
                aflag = 1;
                break;
            case 'b':
                bflag = 1;
                break;
            case 'c':
                cvalue = optarg;
                break;
            case 'd':
                dflag = 1;
                break;
            case 'e':
                evalue = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (optopt == 'e')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                // Character out of char range
                return 1;
            default:
                abort ();
        }

    printf("aflag = %d, bflag = %d, cvalue = %s\n",
            aflag, bflag, cvalue);

    if (dflag)
        printf("avg of the first %d prime numbers = %.2lf\n",
           FIRST_N_PRIMES,
           average_first_n_primes(FIRST_N_PRIMES));

    printf("evalue = %s\n", evalue);

    if (evalue && cvalue) {

        int cval = strtol(cvalue, NULL, 10);
        int eval = strtol(evalue, NULL, 10);

        if ((cval == 0) || (eval == 0)) {
            fprintf (stderr, "Options -c and -e require valid integer argument.\n");
            return 1;
        }

        double  c_avg = average_first_n_primes(cval), e_avg = average_first_n_primes(eval);
        printf("AVG(%.3lf, %.3lf) = %.3lf", c_avg, e_avg, (c_avg+e_avg)/2);
    }

    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    return 0;

}

