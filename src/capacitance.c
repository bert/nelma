#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "assert.h"
#include "lists.h"
#include "data.h"
#include "object.h"
#include "error.h"
#include "dump.h"
#include "sor.h"
#include "space.h"
#include "malloc.h"

struct result {
	n_float c;
	struct net *net1, *net2;
};

static struct result **results;
static n_int resultnum;

int a_standoff=50;
int a_iterations=100;
int a_dump=0;
n_float a_maxerror=0.10;

int a_restore=0;
int a_interrupt=0;

/* FIXME: move result handling logic to a separate module: this isn't specific
 * to capacitance calculation */

static int cap_save_results(char *filename)
{
	FILE *f;

	int m,n;

	f=fopen(filename, "w");
	if(f==NULL) {
		error("Can't open %s: %s", filename, strerror(errno));
		return -1;
	}

	for(n=0;n<resultnum;n++) {
		for(m=0;m<resultnum;m++) {
			fprintf(f, "%e ", results[n][m].c);

			if(results[n][m].net1==NULL) {
				fprintf(f, " _NULL_");
			} else {
				fprintf(f, " %s", results[n][m].net1->name);
			}

			if(results[n][m].net2==NULL) {
				fprintf(f, " _NULL_\n");
			} else {
				fprintf(f, " %s\n", results[n][m].net2->name);
			}
		}
	}

	fclose(f);

	return 0;
}

static int cap_load_results(char *filename)
{
	FILE *f;
	char netname1[1024], netname2[1024];

	int m,n;

	f=fopen(filename, "r");
	if(f==NULL) {
		error("Can't open %s: %s", filename, strerror(errno));
		return -1;
	}

	for(n=0;n<resultnum;n++) {
		for(m=0;m<resultnum;m++) {
			fscanf(f, "%e %s %s", &results[n][m].c, 
							netname1, netname2);

			if(!strcmp(netname1, "_NULL_")) {
				results[n][m].net1=NULL;
			} else {
				results[n][m].net1=net_find(netname1);
			}

			if(!strcmp(netname2, "_NULL_")) {
				results[n][m].net2=NULL;
			} else {
				results[n][m].net2=net_find(netname2);
			}
		}
	}

	fclose(f);

	return 0;
}

static int cap_alloc_results()
{
	int n,m;
	struct net *net;

	resultnum=0;

	net=net_list;
	while(net!=NULL) {
		resultnum++;
		net=net->next;
	}

	results=calloc(resultnum, sizeof(*results));

	if(results==NULL) return -1;

	for(n=0;n<resultnum;n++) {
		results[n]=calloc(resultnum, sizeof(*results[n]));
		if(results[n]==NULL) return -1;

		for(m=0;m<resultnum;m++) {
			results[n][m].net1=NULL;
			results[n][m].net2=NULL;
		}
	}

	return 0;
}

static void cap_free_results()
{
	int n;

	for(n=0;n<resultnum;n++) {
		free(results[n]);
	}
	free(results);
}

#define OBJ_IS_NET(obj)	((obj->role==net)&&(obj->mat->type==metal))

n_float face_flow(struct face *f, struct space *sp)
{
	n_v3i p, p1, p2;
	n_v3i pos;

	n_float h, a, b;
	n_float e1, e2, e3, e4, ex;
	n_float flow;

	pos=f->pos;

	if(pos.x <= sp->pos.x) return 0.0;
	if(pos.x >= sp->pos.x + sp->size.x - 1) return 0.0;

	if(pos.y <= sp->pos.y) return 0.0;
	if(pos.y >= sp->pos.y + sp->size.y - 1) return 0.0;

	// debug("lay: %s mat: %s e: %e", lay->name, mat->name, mat->e);

	h=fabs(f->n.x * sp->step.x + f->n.y * sp->step.y + f->n.z * sp->step.z);

	a=f->e1.x * sp->step.x + f->e1.y * sp->step.y + f->e1.z * sp->step.z;
	b=f->e2.x * sp->step.x + f->e2.y * sp->step.y + f->e2.z * sp->step.z;

	p=pos;
	p1=v3i_add(p, f->n);
	p2=v3i_sub(p, f->n);
	e1=(sp_n_get(sp, p1) - sp_n_get(sp, p2))/2/h;

	p=v3i_add(pos, f->e1);
	p1=v3i_add(p, f->n);
	p2=v3i_sub(p, f->n);
	e2=(sp_n_get(sp, p1) - sp_n_get(sp, p2))/2/h;

	p=v3i_add(pos, f->e2);
	p1=v3i_add(p, f->n);
	p2=v3i_sub(p, f->n);
	e3=(sp_n_get(sp, p1) - sp_n_get(sp, p2))/2/h;

	p=v3i_add(pos, f->e1);
	p=v3i_add(p, f->e2);
	p1=v3i_add(p, f->n);
	p2=v3i_sub(p, f->n);
	e4=(sp_n_get(sp, p1) - sp_n_get(sp, p2))/2/h;

	ex=(e1+e2+e3+e4)/4;

	flow=ex * sp_a_get(sp, pos) * a * b;

	return flow;
}

n_float face_flow_sum(struct face *list, struct space *sp)
{
	struct face *cur;
	n_float sum,h;

	/*
	n_float fgx1,fgy1,fgz1;
	n_float fgx2,fgy2,fgz2;

	fgx1=0.0;
	fgy1=0.0;
	fgz1=0.0;

	fgx2=0.0;
	fgy2=0.0;
	fgz2=0.0;
	*/

	sum=0.0;


	cur=list;
	while(cur!=NULL) {
		h=face_flow(cur, sp);

		/*
		if(cur->n.x==1.0) {
			fgx2+=h;
		} else if(cur->n.x==-1.0) {
			fgx1+=h;
		} else if(cur->n.y==1.0) {
			fgy2+=h;
		} else if(cur->n.y==-1.0) {
			fgy1+=h;
		} else if(cur->n.z==1.0) {
			fgz2+=h;
		} else if(cur->n.z==-1.0) {
			fgz1+=h;
		} else {
			assert(0);
		}
		*/

		sum=sum+h;
		cur=cur->next;
	}

	/*
	debug("\npos x: %e, neg x: %e", fgx2, fgx1);
	debug("pos y: %e, neg y: %e", fgy2, fgy1);
	debug("pos z: %e, neg z: %e", fgz2, fgz1);

	sum=fgx1+fgx2+fgy1+fgy2+fgz1+fgz2;
	*/

	return sum;
}

static void cap_dump_sp(struct space *sp, char *name)
{
	char dumpfile[256];
	n_v3i pos;

	struct layer *lay;
	n_int z;
	int n;

	pos=v3i(sp->size.x/2, sp->size.y/2, sp->size.z/2);
	pos=v3i_add(pos, sp->pos);

	strcpy(dumpfile, name);
	strcat(dumpfile, "-x.dat");
	sp_dump(sp, X, pos, dumpfile);
	strcpy(dumpfile, name);
	strcat(dumpfile, "-y.dat");
	sp_dump(sp, Y, pos, dumpfile);

	for(n=0;n<sp->laynum;n++) {
		lay=sp->lay[n];

		strcpy(dumpfile, name);
		strcat(dumpfile, "-");
		strcat(dumpfile, lay->name);
		strcat(dumpfile, ".dat");

		z=lay->z+lay->height/2;

		pos=v3i(sp->size.x/2, sp->size.y/2, z);
		pos=v3i_add(pos, sp->pos);

		sp_dump(sp, Z, pos, dumpfile);
	}
}

static void cap_one(struct space *sp, struct net *net, struct result *r)
{
	struct net *cur;
	struct object *cp;
	struct face *f;

	n_float error, max_error;

	n_float q;

	n_v2i pos, size;

	int n, iterations;

	cur=net_list;
	while(cur!=NULL) {
		if(cur==net) {
			net_set(cur, 1.0, 1);
		} else {
			net_set(cur, 0.0, 1);
		}

		cur=cur->next;
	}

	info("Evaluating net %s", net->name);
	info("Standoff = %d", a_standoff);

	cp=net_get_composite(net);
	obj_grow(cp, a_standoff, GROW_ROUND);

	pos=cp->pos;
	size=cp->size;

	sp_load(sp, pos, v2i_add(size, v2i(1, 1)));

	cp->con=0;
	cp->n=0.0;

	// obj_invert(cp);

	for(n=0;n<sp->laynum;n++) {
		sp_add_obj(sp, cp, sp->lay[n]);
	}

	obj_unload(cp);
	obj_done(cp);

	info("Grid is size (%d, %d, %d) and position (%d, %d, %d)", 
					sp->size.x, sp->size.y, sp->size.z,
					sp->pos.x, sp->pos.y, 0);

	sp_add_obj_all(sp);

	// sp_clear(sp);
	sp_border(sp);

	sp_optimize(sp);

	// sp_optimize(sp);

	mem_info();

	iterations=0;
	while(1) {
		for(n=0;n<a_iterations;n++) {
			sor_iterate(sp);
			iterations++;

			fprintf(stderr, ".");
			fflush(stderr);
		}
		fprintf(stderr, "o");
		fflush(stderr);

		max_error=-1.0;

		n=0;
		cur=net_list;
		while(cur!=NULL) {
			f=net_get_face_sp(cur, sp, 1);

			// face_dump(f, cur->name); 

			q=face_flow_sum(f, sp);

			r[n].net1=net;
			r[n].net2=cur;

			//printf("%le\n", q);

			if(q!=0.0) {
				error=fabs((r[n].c-q)/q);
				if(error>max_error) max_error=error;
			}

			r[n].c=q;

			face_done(f);

			n++;

			cur=cur->next;
		}

		fprintf(stderr, "[%4.2f]", max_error);
		fflush(stderr);

		if(max_error<a_maxerror) break;
	}

	fprintf(stderr, "\n");
	fflush(stderr);

	info("Finished after total %d iterations", iterations);

	if(a_dump) {
		cap_dump_sp(sp, net->name);
	}

	/*
	for(n=0;n<sp->laynum;n++) {
		lay_dump(sp->lay[n], sp->lay[n]->name);
	}
	*/

	sp_unload(sp);
}

int cap_main()
{
	struct net *net;
	n_float c,d;
	int n,m;

	if(cap_alloc_results()) {
		return -1;
	}

	if(a_restore) {
		cap_load_results("nelma.save");
	}

	n=0;
	net=net_list;
	while(net!=NULL) {
		if(results[n][0].net1==NULL) {
			cap_one(c_space, net, results[n]);
		}
		n++;
		net=net->next;

		if(a_interrupt) {
			cap_save_results("nelma.save");
			cap_free_results();
			return 0;
		}
	}

	for(n=0;n<resultnum;n++) {
		for(m=0;m<resultnum;m++) if(n<m) {
			c=(results[n][m].c+results[m][n].c)/2;
			d=fabs(results[n][m].c-c);
			printf("C%02d%02d %s %s %e\n", n, m, 
						results[n][m].net1->name, 
						results[n][m].net2->name, c);						
			/*
			printf("* %s -> %s %e\n", results[n][m].net1->name,
						  results[n][m].net2->name, 
						  results[n][m].c);
			printf("* %s -> %s %e\n", results[m][n].net1->name,
						  results[m][n].net2->name, 
						  results[m][n].c);
			*/
			printf("* numerical error: +/- %.2e (%5.0f %%)\n", 
						d, d/c*100.0);
		}
	}

	cap_free_results();

	return 0;
}
