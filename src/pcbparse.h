#ifndef _PCBPARSE_H
#define _PCBPARSE_H

#include "listutil.h"

/** @brief Structure describing a pin on a pcb board */
struct pcbpin {
	/** @brief Position of the center of the pin in pcb 
	 * coordinates. */
	n_v2i pos;

	/** @brief Size of the copper ring in pcb units. */
	n_int copper;
	/** @brief Size of the hole in pcb units. */
	n_int hole;

	/** @brief Name of the pin in "element-pin" format. */
	char *name;

	/** @brief Pointer to the next struct in linked list. */
	struct pcbpin *next;
};

struct pcbnet {
	char *name;

	struct pcbpin **pin;
	int pinnum;

	/** @brief Pointer to the next struct in linked list. */
	struct pcbnet *next;
};

LIST_NEW_H_NAME(pin, pcbpin);
LIST_NEW_H_NAME(net, pcbnet);
	
#endif
