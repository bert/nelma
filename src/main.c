#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "assert.h"
#include "data.h"
#include "error.h"
#include "lists.h"
#include "config.h"
#include "capacitance.h"
#include "sor.h"

char *a_configfile=NULL;

void main_interrupt(int num)
{
	error("\nInterrupt. Will finish this net and save state. Use '-r' to restore.");

	a_interrupt=1;

	signal(SIGINT, SIG_DFL);
}

void main_syntax()
{
	printf("NELMA, Numerical ElectroMagnetics. Capacitance calculator version %s\n", VERSION);
	printf("Copyright (c) 2006 Tomaz Solc\n\n");

	printf("SYNTAX: nelma-cap [ -s STANDOFF ]\n");
	printf("                  [ -n ITERATIONS ]\n");
	printf("                  [ -w SOR_OMEGA ]\n");
	printf("                  [ -e MAX_ERROR_%% ]\n");
	printf("                  [ -d ]\n");
	printf("                  [ -v VERBOSITY ]\n");
	printf("                  [ -r ]\n");
	printf("                  config_file.em\n\n");

	printf("Bug reports to <tomaz.solc@tablix.org>\n");

	return;
}

void main_header(int argc, char **argv)
{
	int n;
	time_t t;

	t=time(NULL);

	printf("* Created by NELMA capacitance calculator, version %s\n", VERSION);
	printf("* %s", ctime(&t));
	printf("*\n");
	printf("* Command line:");

	for(n=0;n<argc;n++) {
		printf(" %s", argv[n]);
	}
	printf("\n\n");
}

void main_args(int argc, char **argv)
{
	int c,r;

        while ((c=getopt(argc, argv, "hs:n:dv:w:e:r"))!=-1) {
                switch (c) {
			case 'e': r=sscanf(optarg, "%f", &a_maxerror);
				  if(r!=1) {
				  	error("Invalid max error setting '%s'",
								optarg);
				  }
				  break;
			case 'w': r=sscanf(optarg, "%f", &a_soromega);
				  if(r!=1) {
				  	error("Invalid omega setting '%s'",
								optarg);
				  }
				  break;
			case 'v': r=sscanf(optarg, "%d", &a_verbosity);
				  if(r!=1) {
				  	error("Invalid verbosity setting '%s'",
								optarg);
				  }
				  break;
			case 'n': r=sscanf(optarg, "%d", &a_iterations);
				  if(r!=1) {
				  	error("Invalid iterations setting '%s'",
								optarg);
				  }
				  break;
			case 's': r=sscanf(optarg, "%d", &a_standoff);
				  if(r!=1) {
				  	error("Invalid standoff setting '%s'",
								optarg);
				  }
				  break;
                        case 'd': a_dump=1;
                                  break;
			case 'r': a_restore=1;
			          break;
			case 'h': main_syntax();
				  return;
			default:
				  error("Wrong command line arguments (try -h)\n");
				  return;
                }
        }

	if(!(optind<argc)) {
		error("Missing config file (try -h)");
		return;
	}

	a_configfile=argv[optind];
}

int main(int argc, char **argv) 
{
	int r;

	main_args(argc, argv);

	if(a_configfile==NULL) return 1;

	r=parse_main(a_configfile);
	
	if(r) {
		error("Can't parse config file %s", a_configfile);
		return 1;
	}

	main_header(argc, argv);

        signal(SIGINT, main_interrupt);

	cap_main();

	return 0;
}
