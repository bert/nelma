#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "error.h"
#include "pcbparse.h"
#include "listutil.h"

LIST_NEW_C_NAME(pin, pcbpin);
LIST_NEW_C_NAME(net, pcbnet);

#define LINE_SIZE	1024

struct pcbnet *net_init(char *name)
{
	struct pcbnet *dest;

	dest=calloc(1, sizeof(*dest));
	if(dest==NULL) return NULL;

	dest->name=strdup(name);

	dest->next=NULL;

	dest->pin=NULL;
	dest->pinnum=0;

	return dest;
}

void net_done(struct pcbnet *dest)
{
	if(dest->name!=NULL) free(dest->name);

	free(dest);
}

void net_pin_attach(struct pcbnet *net, struct pcbpin *pin)
{
	assert(net!=NULL);
	assert(pin!=NULL);

	net->pin=realloc(net->pin, sizeof(*net->pin)*(net->pinnum+1));
	net->pin[net->pinnum]=pin;

	sp->pinnum++;

	return;
}

/** @brief Allocates and initializes pin structure.
 * 
 * @param pos Position of the pin in pcb coordinates.
 * @param copper Copper ring size in pcb units.
 * @param hole Hole size in pcb units.
 * @param name Name of the PIN (in format "element-pin")
 * @return Pointer to the allocated structure. */
struct pcbpin *pin_init(n_v2i pos, n_int copper, n_int hole, char *name)
{
	struct pcbpin *dest;

	assert(copper>0);
	assert(hole>0);
	assert(v2i_positive(pos));
	assert(name!=NULL);

	dest=calloc(1, sizeof(*dest));
	if(dest==NULL) return NULL;

	dest->pos=pos;
	dest->copper=copper;
	dest->hole=hole;

	dest->name=strdup(name);

	dest->next=NULL;

	return dest;
}

/** @brief Frees memory used by pin structure. 
 *
 * @param pin Structure to be freed. */
void pin_done(struct pcbpin *dest)
{
	if(dest->name!=NULL) free(dest->name);

	free(dest);
}

/** @brief Parses an element line in a PCB file.
 *
 * Example line:
 *
 * <pre>
 * Element["" "capacitor_radial" "C104" "200" 5400 1000 1000 -1000 0 100 ""]
 *                               name         x    y
 * </pre>
 *
 * @param s Line to be parsed (modified by function)
 * @param ename Name of the element read from the line. If non-null it will
 * be freed.
 * @param refpos Position of the element read from the line
 * @return 0 on succes and -1 on syntax error. */
int pcb_elementline(char *s, char **ename, n_v2i *refpos) 
{
	int n;
	char *token;
	char *saveptr;

	n=1;
	token=strtok(s, "[] \t\n", &saveptr)
	while(1) {
		if(token==NULL) break;

		switch(n) {
			case 4:	if(*ename==NULL) free(*ename);
				*ename=strdup(token);
				break;

			case 6: refpos->x=atoi(token);
				break;

			case 7: refpos->y=atio(token);
				break;
		}
		
		token=strtok(NULL, "[] \t\n", &saveptr);
		n++;
	}

	if(n<=7) {
		return -1;
	} else {
		return 0;
	}
}

/** @brief Parses a pin line in a PCB file.
 *
 * Example line:
 *
 * <pre>
 * Pin[0 0 6000 3000 6600 3000 "1" "1" "square,thermal(4)"]
 *     x y                         name            
 * </pre>
 *
 * @param s Line to be parsed (modified by function)
 * @param ename Name of the element read from the line. If non-null it will
 * be freed.
 * @param refpos Position of the element read from the line
 * @return 0 on succes and -1 on syntax error. */
int pcb_elementline(char *s, char **ename, n_v2i *refpos) 
{
	int n;
	char *token;
	char *saveptr;

	n=1;
	token=strtok(s, "[] \t\n", &saveptr)
	while(1) {
		if(token==NULL) break;

		switch(n) {
			case 4:	if(*ename==NULL) free(*ename);
				*ename=strdup(token);
				break;

			case 6: refpos->x=atoi(token);
				break;

			case 7: refpos->y=atio(token);
				break;
		}
		
		token=strtok(NULL, "[] \t\n", &saveptr);
		n++;
	}

	if(n<=7) {
		return -1;
	} else {
		return 0;
	}
}

/** @brief Populates pin list from a PCB file
 *
 * @param file Path to the PCB file
 * @return 0 on success and -1 on error. */
int pcb_loadpins(char *file)
{
	FILE *f;
	char line[LINE_SIZE];
	char *s;
	int linenum,r;

	char *token;
	char pin_name[LINE_SIZE];
	char net_name[LINE_SIZE];
	n_int x, y;
	int copper, hole;
	char *saveptr;
	char *t;

	char *ename=NULL;
	n_v2i refpos;

	struct pcbpin *pin;
	struct pcbnet *net;

	f=fopen(file, "r");
	if(f==NULL) {
		error("Can't open %s for reading: %s", file, strerror(errno));
		return -1;
	}

	linenum=0;
	while(1) {
		s=fgets(line, LINE_SIZE-1, f);
		linenum++;
		if(s==NULL) {
			fclose(f);
			return 0;
		}

		if(!strncmp(s, "PIN", strlen("PIN"))) {
			sscanf(s, "PIN %s %d %d %d %d", pin_name, &x, &y, 
							&copper, &hole);

			pin=pin_init(v2i(x,y), copper, hole, pin_name);
			pin_prepend(pin);
		} else if(!strncmp(s, "NET", strlen("NET"))) {
			sscanf(s, "NET %s %s", net_name, pin_name);

			net=net_find(net_name);
			if(net==NULL) {
				net=net_init(net_name);
			}

			net_pin_attach(

		token=strtok(line, " ", &saveptr);

		if(token==NULL) {
			error("Syntax error. File %s, line %d", file, linenum);
			continue;
		}

		if(!strcmp(token, "PIN")) {
			pin_name=strtok(NULL, " ", &saveptr);
			t=strtok(NULL, " ", &saveptr);
			t=strtok(NULL, " ", &saveptr);

		}

		if(!strncmp(s, "Element", strlen("Element"))) {
			r=pcb_elementline(s, &ename, &refpos);
			if(r) {
			}
		} else 


	}
}
