/*
	Group 6 Final Project
	
	llc - last level cache simulator
	
	usage:  llc [filename] [-s]
	
	if no filename given, reads standard input.
	"Normal" processing produces output for each
	line of input; -s turns	off all but statistics
	output.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELEN 20

int NormalMode = 1;

void ParseFile(FILE *input)
{
	char cmd;
	unsigned int addr;
	
	char *line;
	char buf[MAXLINELEN];
	
	while ((line = fgets(buf, MAXLINELEN, input)) != NULL)
	{
		if (sscanf(line, " %c %x", &cmd, &addr) != 2)
		{
			fprintf(stderr, "bad input line ignored: '%s'\n", line);
			continue;
		}
		
		/*
		if (strlen(tmp) != 1)
		{
			fprintf(stderr, "bad command in input line: '%s'\n", tmp);
			continue;
		}
		*/
		
		/* check that cmd is a valid char */
		/* cmd = *tmp; */
		
		switch (cmd)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '8':
				/* Not yet implemented */
				break;
					
			case '9':
				/* Print statistics */
				break;
					
			default:
				fprintf(stderr, "bad command character: '%c'\n", cmd);
				continue;
		}

		/* Check that the address is valid */
		/* if (addr > MAXADDRESS) ... */
			
		if (NormalMode)
		{
			printf("BusOp: %c Address: %08x\n", cmd, addr);
		}
	}
}


int main(int argc, char *argv[])
{	
	FILE *file;
	char *filename = NULL;
	
	for (int i = 1; i < argc; i++)
	{
		if (!strcasecmp(argv[i], "-s"))
		{
			NormalMode = 0;
		}
		else if (filename == NULL)
		{
			filename = argv[i];
		}
		else
		{
			printf("Usage: llc filename [-s]\n");
			exit(1);
		}
    }

	/* If no file given, read from standard input */
	if (filename == NULL || !strcasecmp(filename, "-"))
	{
		file = stdin;
	}
	else
    {
        file = fopen(filename, "r");
 
        if (!file)
        {
            fprintf(stderr, "Could not open file %s\n", filename);
			exit(1);
        }
	}

	ParseFile(file);
}