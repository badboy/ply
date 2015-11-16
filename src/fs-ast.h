#ifndef __FS_AST_H
#define __FS_AST_H

#include <assert.h>
#include <search.h>
#include <stdint.h>
#include <stdlib.h>

static inline void insque_tail(void *elem, void *prev)
{
	struct { void *next, *prev; } *le = elem, *pe = prev;

	for (; pe && pe->next; pe = pe->next);
	insque(le, pe);
}

struct fs_map {
	struct fs_node *vargs;
};

struct fs_binop {
	struct fs_node *left, *right;
};

struct fs_assign {
	struct fs_node *lval, *expr;
};

struct fs_cond {
	struct fs_node *cond;
	struct fs_node *yes, *no;
};

struct fs_call {
	struct fs_node *vargs;
};

struct fs_probe {
	struct fs_node *pspecs;
	struct fs_node *stmts;
};

struct fs_script {
	struct fs_node *probes;
};

#define FS_TYPE_TABLE \
	TYPE(FS_UNKNOWN, "UNKNOWN")		\
	TYPE(FS_SCRIPT, "script")		\
	TYPE(FS_PROBE, "probe")			\
	TYPE(FS_PSPEC, "pspec")			\
	TYPE(FS_CALL, "call")			\
	TYPE(FS_COND, "cond")			\
	TYPE(FS_ASSIGN, "assign")		\
	TYPE(FS_RETURN, "return")		\
	TYPE(FS_BINOP, "binop")			\
	TYPE(FS_NOT, "not")			\
	TYPE(FS_MAP, "map")			\
	TYPE(FS_VAR, "var")			\
	TYPE(FS_INT, "int")			\
	TYPE(FS_STR, "str")

#define TYPE(_type, _typestr) _type,
enum fs_type {
	FS_TYPE_TABLE
};
#undef TYPE

const char *fs_typestr(enum fs_type type);

enum fs_location {
	LOC_NOWHERE,
	LOC_REG,
	LOC_STACK,
};

struct fs_annot {
	enum fs_type     type;
	size_t           size;

	enum fs_location loc;
	int              reg;
	ssize_t          addr;
};

static inline int fs_annot_compatible(struct fs_annot *a, struct fs_annot *b)
{
	return a->type == b->type && a->size == b->size;
}

struct fs_node {
	void *next, *prev;

	enum fs_type type;

	char *string;

	union {
		struct fs_script script;
		struct fs_probe  probe;
		struct fs_call   call;
		struct fs_cond   cond;
		struct fs_assign assign;
		struct fs_binop  binop;
		struct fs_map    map;
		struct fs_node  *not;
		struct fs_node  *ret;
		
		int64_t          integer;
	};

	struct fs_annot annot;
};


#define fs_foreach(_n, _in) for((_n) = (_in); (_n); (_n) = (_n)->next)

static inline struct fs_node *fs_node_new(enum fs_type type) {
	struct fs_node *n = calloc(1, sizeof(*n));

	assert(n);
	n->type = type;
	return n;
}

void fs_ast_dump(struct fs_node *n);

struct fs_node *fs_str_new(char *val);
struct fs_node *fs_int_new(int64_t val);
struct fs_node *fs_var_new(char *name);
struct fs_node *fs_map_new(char *name, struct fs_node *vargs);
struct fs_node *fs_not_new(struct fs_node *expr);
struct fs_node *fs_return_new(struct fs_node *expr);
struct fs_node *fs_binop_new(struct fs_node *left, char *op, struct fs_node *right);
struct fs_node *fs_assign_new(struct fs_node *lval, char *op, struct fs_node *expr);
struct fs_node *fs_cond_new(struct fs_node *cond,
			    struct fs_node *yes, struct fs_node *no);
struct fs_node *fs_call_new(char *func, struct fs_node *vargs);
struct fs_node *fs_pspec_new(char *spec);
struct fs_node *fs_probe_new(struct fs_node *pspecs, struct fs_node *stmts);
struct fs_node *fs_script_new(struct fs_node *probes);

void fs_free(struct fs_node *n);
int  fs_walk(struct fs_node *n,
	     int  (*pre)(struct fs_node *n, void *ctx),
	     int (*post)(struct fs_node *n, void *ctx), void *ctx);

#endif	/* __FS_AST_H */
