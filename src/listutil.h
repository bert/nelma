#define LIST_H(_prefix_, _struct_)					\
		extern struct _struct_ *_prefix_ ## _list;

#define LIST_C(_prefix_, _struct_)					\
		struct _struct_ *_prefix_ ## _list=NULL;

#define FIND_FUNC_H(_prefix_, _struct_) 				\
		struct _struct_ * _prefix_ ## _find(char *name);

#define FIND_FUNC_C(_prefix_, _struct_) 				\
		struct _struct_ * _prefix_ ## _find(char *name)		\
		{							\
			struct _struct_ *cur;				\
									\
			if(name==NULL) return NULL;			\
									\
			cur=_prefix_ ## _list;				\
									\
			while(cur!=NULL) {				\
				if(!strcmp(cur->name, name)) {		\
					return cur;			\
				}					\
				cur=cur->next;				\
			}						\
									\
			return NULL;					\
		}


#define PREPEND_FUNC_H(_prefix_, _struct_) 				\
		void _prefix_ ## _prepend(struct _struct_ *arg);

#define PREPEND_FUNC_C(_prefix_, _struct_) 				\
		void _prefix_ ## _prepend(struct _struct_ *arg) 	\
		{							\
			arg->next=_prefix_ ## _list;			\
			_prefix_ ## _list=arg; 				\
									\
			return;						\
		}

#define DELETE_FUNC_H(_prefix_, _struct_)				\
		void _prefix_ ## _delete(struct _struct_ *arg);

#define DELETE_FUNC_C(_prefix_, _struct_)				\
		void _prefix_ ## _delete(struct _struct_ *arg)		\
		{							\
			struct _struct_ *cur, *prev;			\
									\
			cur=_prefix_ ## _list;				\
			prev=NULL;					\
									\
			while(cur!=NULL) {				\
				if(cur==arg) {				\
					if(prev!=NULL) {		\
						prev->next=cur->next;	\
					} else {			\
						_prefix_ ## _list=cur->next; \
					}				\
					return;				\
				}					\
									\
				prev=cur;				\
				cur=cur->next;				\
			}						\
									\
			return;						\
		}

#define LIST_NEW_H(_prefix_, _struct_)					\
		LIST_H(_prefix_, _struct_)				\
		PREPEND_FUNC_H(_prefix_, _struct_)			\
		DELETE_FUNC_H(_prefix_, _struct_)			\

#define LIST_NEW_C(_prefix_, _struct_)					\
		LIST_C(_prefix_, _struct_)				\
		PREPEND_FUNC_C(_prefix_, _struct_)			\
		DELETE_FUNC_C(_prefix_, _struct_)			\

#define LIST_NEW_H_NAME(_prefix_, _struct_)				\
		LIST_H(_prefix_, _struct_)				\
		PREPEND_FUNC_H(_prefix_, _struct_)			\
		DELETE_FUNC_H(_prefix_, _struct_)			\
		FIND_FUNC_H(_prefix_, _struct_)

#define LIST_NEW_C_NAME(_prefix_, _struct_)				\
		LIST_C(_prefix_, _struct_)				\
		PREPEND_FUNC_C(_prefix_, _struct_)			\
		DELETE_FUNC_C(_prefix_, _struct_)			\
		FIND_FUNC_C(_prefix_, _struct_)
