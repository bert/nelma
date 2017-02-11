#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lists.h"
#include "data.h"
#include "loadobj.h"
#include "error.h"
#include "confuse.h"
#include "assert.h"
#include "object.h"
#include "space.h"

static cfg_opt_t opts_net[] = {
	CFG_STR_LIST("objects", NULL, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t opts_object[] = {
	CFG_INT_LIST("position", NULL, CFGF_NODEFAULT),
	CFG_INT_LIST("size", NULL, CFGF_NODEFAULT),
	CFG_INT("radius", 0, CFGF_NODEFAULT),
	CFG_STR("material", NULL, CFGF_NODEFAULT),
	CFG_STR("type", NULL, CFGF_NODEFAULT),
	CFG_STR("file", NULL, CFGF_NODEFAULT),
	CFG_STR("role", NULL, CFGF_NODEFAULT),
	CFG_INT_LIST("file-pos", NULL, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t opts_material[] = {
	CFG_STR("type", "dielectric", CFGF_NONE),
	CFG_FLOAT("permittivity", 8.85e-12, CFGF_NONE),
	CFG_FLOAT("conductivity", 0.0, CFGF_NONE),
	CFG_FLOAT("permeability", 0.0, CFGF_NONE),
	CFG_END()
};

static cfg_opt_t opts_layer[] = {
	CFG_INT("height", -1, CFGF_NONE),
	CFG_INT("z-order", -1, CFGF_NONE),
	CFG_STR_LIST("objects", NULL, CFGF_NODEFAULT),
	CFG_STR("material", NULL, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t opts_space[] = {
	CFG_FLOAT_LIST("step", NULL, CFGF_NODEFAULT),
	CFG_STR_LIST("layers", NULL, CFGF_NODEFAULT),
	CFG_END()
};

static cfg_opt_t opts_root[] = {
	CFG_SEC("object", opts_object, CFGF_MULTI|CFGF_TITLE),
	CFG_SEC("material", opts_material, CFGF_MULTI|CFGF_TITLE),
	CFG_SEC("layer", opts_layer, CFGF_MULTI|CFGF_TITLE),
	CFG_SEC("space", opts_space, CFGF_MULTI|CFGF_TITLE),
	CFG_SEC("net", opts_net, CFGF_MULTI|CFGF_TITLE),
	CFG_END()
};

/*
static n_v2f cfg_get_v2f(cfg_t *cfg, const char *name)
{
	n_v2f r;

	r.x=cfg_getnfloat(cfg, name, 0);
	r.y=cfg_getnfloat(cfg, name, 1);

	return r;
}
*/

static n_v3f cfg_get_v3f(cfg_t *cfg, const char *name)
{
	n_v3f r;

	if(cfg_size(cfg, name)!=3) return v3f(-1,-1,-1);

	r.x=cfg_getnfloat(cfg, name, 0);
	r.y=cfg_getnfloat(cfg, name, 1);
	r.z=cfg_getnfloat(cfg, name, 2);

	return r;
}

static n_axis cfg_get_axis(cfg_t *cfg, const char *name)
{
	if(!strcasecmp(cfg_getstr(cfg, name), "x")) {
		return X;
	} else if(!strcasecmp(cfg_getstr(cfg, name), "y")) {
		return Y;
	} else {
		return Z;
	}
}

static n_v2i cfg_get_v2i(cfg_t *cfg, const char *name)
{
	n_v2i r;

	if(cfg_size(cfg, name)!=2) return v2i(-1,-1);

	r.x=cfg_getnint(cfg, name, 0);
	r.y=cfg_getnint(cfg, name, 1);

	return r;
}

static n_v2i *cfg_get_v2i_array(cfg_t *cfg, const char *name)
{
	int n,m;
	n_v2i *r;

	n=cfg_size(cfg,name);
	if(((n%2)!=0)||(n<2)) return NULL;

	r=calloc(n/2+1, sizeof(*r));
	if(r==NULL) return NULL;

	for(m=0;m<n/2;m++) {
		r[m].x=cfg_getnint(cfg, name, m*2);
		r[m].y=cfg_getnint(cfg, name, m*2+1);
	}

	r[m].x=-1;
	r[m].y=-1;

	return r;
}

static n_v3i cfg_get_v3i(cfg_t *cfg, const char *name)
{
	n_v3i r;

	if(cfg_size(cfg, name)!=3) return v3i(-1,-1, -1);

	r.x=cfg_getnint(cfg, name, 0);
	r.y=cfg_getnint(cfg, name, 1);
	r.z=cfg_getnint(cfg, name, 2);

	return r;
}

static void parse_net(cfg_t *cfg) 
{
	const char *name;

	char *objname;

	struct object *obj;

	int n,i;

	struct net *net;

	net=net_init();

	name=cfg_title(cfg);
	net->name=strdup(name);

	n=cfg_size(cfg, "objects");
	for(i=0;i<n;i++) {
		objname=cfg_getnstr(cfg, "objects", i);
		obj=obj_find(objname);
		if(obj==NULL) {
			error("net %s: unknown object %s", net->name, objname);
			net_done(net);
			return;
		}
		net_obj_attach(net, obj);
	}

	net_prepend(net);

	info("Net '%s': %d objects", net->name, n);

	return;
}

static void parse_layer(cfg_t *cfg) 
{
	n_int height, order;
	struct material *mat;
	const char *name;

	char *objname;

	struct layer *lay;
	struct object *obj;

	int n,i;

	height=cfg_getint(cfg, "height");
	if(height<=0) {
		error("layer: %s: invalid height %d", cfg_title(cfg), height);
		return;
	}

	order=cfg_getint(cfg, "z-order");
	if(order<0) {
		error("layer: %s: invalid z-order %d", cfg_title(cfg), order);
		return;
	}

	mat=mat_find(cfg_getstr(cfg, "material"));
	if(mat==NULL) {
		error("layer %s: unknown material %s", 
				cfg_title(cfg), cfg_getstr(cfg, "material"));
		return;
	}

	lay=lay_init(height, order, mat);

	name=cfg_title(cfg);
	lay->name=strdup(name);

	n=cfg_size(cfg, "objects");
	for(i=0;i<n;i++) {
		objname=cfg_getnstr(cfg, "objects", i);
		obj=obj_find(objname);
		if(obj==NULL) {
			error("layer %s: unknown object %s",
							lay->name, objname);
			lay_done(lay);
			return;
		}
		lay_obj_attach(lay, obj);
	}

	lay_prepend(lay);

	info("Layer '%s': height=%d z-order=%d material=%s", 
			lay->name, lay->height, lay->order, lay->mat->name);

	return;
}

static void parse_space(cfg_t *cfg) 
{
	n_v3f step;
	const char *name;

	char *layname;

	struct layer *lay;
	struct space *sp;

	int n,i;

	if(c_space!=NULL) {
		error("More than one space specified in the config file");
		return;
	}

	step=cfg_get_v3f(cfg, "step");

	sp=sp_init(step);

	name=cfg_title(cfg);
	sp->name=strdup(name);
	
	n=cfg_size(cfg, "layers");
	for(i=0;i<n;i++) {
		layname=cfg_getnstr(cfg, "layers", i);

		lay=lay_find(layname);
		if(lay==NULL) {
			error("space %s: unknown layer %s", 
					sp->name, layname);
			return;
		}

		sp_lay_attach(sp, lay);
	}

	c_space=sp;

	info("Space '%s': step=(%.2e,%.2e,%.2e)",
		sp->name, step.x, step.y, step.z);

	return;
}

static void parse_object(cfg_t *cfg) 
{
	n_v2i pos;
	struct material *mat;
	const char *name;
	char *type, *role;

	struct object *obj;

	pos=cfg_get_v2i(cfg, "position");

//	if(!v2i_positive(pos)) {
//		error("object %s: invalid position", cfg_title(cfg));
//		return;
//	}

	mat=mat_find(cfg_getstr(cfg, "material"));
	if(mat==NULL) {
		error("object %s: unknown material %s", 
				cfg_title(cfg), cfg_getstr(cfg, "material"));
		return;
	}

	type=cfg_getstr(cfg, "type");
	if(type==NULL) {
		error("object %s: type missing", cfg_title(cfg));
		return;
	}

	obj=obj_init(pos, mat);

	if(obj==NULL) {
		error("Can't create object");
		return;
	}

	if(!strcmp(type, "rectangle")) {
		obj->type=rectangle;
		obj->size=cfg_get_v2i(cfg, "size");
	} else if(!strcmp(type, "circle")) {
		obj->type=circle;
		obj->radius=cfg_getint(cfg, "radius");
	} else if(!strcmp(type, "image")) {
		obj->type=image;

		if(cfg_getstr(cfg, "file")==NULL) {
			error("File name not specified for image object %s",
								cfg_title(cfg));
			return;
		}
		obj->image=strdup(cfg_getstr(cfg, "file"));

		obj->imgpos=cfg_get_v2i_array(cfg, "file-pos");
		if(obj->imgpos==NULL) {
			error("Invalid array of image positions in object %s", 
								cfg_title(cfg));
		}
	} else {
		error("Unknown object type %s", type);
		obj_done(obj);
		return;
	}

	obj_load(obj);

	assert(obj->map!=NULL);

	obj_unload(obj);

	name=cfg_title(cfg);
	obj->name=strdup(name);

	role=cfg_getstr(cfg, "role");
	if(role==NULL) {
		error("object %s: role missing", obj->name);
		return;
	}

	if(!strcmp(role, "pin")) {
		obj->role=pin;
	} else if(!strcmp(role, "net")) {
		obj->role=net;
	} else if(!strcmp(role, "none")) {
		obj->role=none;
	} else {
		error("object: %s: unknown role %s", obj->name, role);
		return;
	}

	obj_prepend(obj);

	info("Object '%s' (%s): pos=(%d,%d) size=(%d,%d) "
			     "material=%s", obj->name, cfg_getstr(cfg, "role"),
			     obj->pos.x, obj->pos.y, 
			     obj->size.x, obj->size.y, obj->mat->name);

	return;
}

static void parse_material(cfg_t *cfg) 
{
	struct material *mat;
	n_float e,g,u;
	const char *name;
	char *type;

	type=cfg_getstr(cfg, "type");

	e=cfg_getfloat(cfg, "permittivity");
	g=cfg_getfloat(cfg, "conductivity");
	u=cfg_getfloat(cfg, "permeability");

	if(!strcmp(type, "metal")) {
		mat=mat_init(metal, e, g, u);
	} else if(!strcmp(type, "dielectric")) {
		mat=mat_init(dielectric, e, g, u);
	} else {
		error("material %s: unknown type %s", cfg_title(cfg), type);
		return;
	}

	name=cfg_title(cfg);
	mat->name=strdup(name);

	mat_prepend(mat);

	info("Material '%s': type=%s e=%.2e g=%.2e u=%.2e",
						mat->name, type, e, g, u);

	return;
}

int parse_main(char *file)
{
	cfg_t *cfg;
	int r;
	int n,i;

	cfg=cfg_init(opts_root, CFGF_NONE);

	r=cfg_parse(cfg, file);
	if(r==CFG_FILE_ERROR) {
		error("Can't open %s: %s", file, strerror(errno));
		return -1;
	} else if(r==CFG_PARSE_ERROR) {
		error("Syntax error in %s", file);
		return -1;
	}

	n=cfg_size(cfg, "material");
	for(i=0;i<n;i++) {
		parse_material(cfg_getnsec(cfg, "material", i));
	}

	n=cfg_size(cfg, "object");
	for(i=0;i<n;i++) {
		parse_object(cfg_getnsec(cfg, "object", i));
	}

	n=cfg_size(cfg, "net");
	for(i=0;i<n;i++) {
		parse_net(cfg_getnsec(cfg, "net", i));
	}

	n=cfg_size(cfg, "layer");
	for(i=0;i<n;i++) {
		parse_layer(cfg_getnsec(cfg, "layer", i));
	}

	n=cfg_size(cfg, "space");
	for(i=0;i<n;i++) {
		parse_space(cfg_getnsec(cfg, "space", i));
	}

	cfg_free(cfg);

	return 0;
}
