#ifndef __TCL_H__
#define __TCL_H__

#ifdef __cplusplus
extern "C" {
#endif

struct tcl;

/* Token type and control flow constants */
typedef enum {
  TCMD,
  TWORD,
  TPART,
  TERROR,

  TTVAR,
  TTLIST,
  TTATOM,
  TTQUOTE,
} tcl_token_t;

typedef enum {
  FERROR,
  FNORMAL,
  FRETURN,
  FBREAK,
  FAGAIN
} tcl_status_t;

struct tcl_word_t {
  char *str;
  uint8_t len;
};

typedef struct tcl_word_t tcl_word_t;

/* A helper parser struct and macro (requires C99) */
struct tcl_parser {
  const char* from;
  const char* to;
  const char* start;
  const char* end;
  int q;
  int token;
};
#define tcl_each(s, len, skiperr)                                                                                          \
  for (struct tcl_parser p = { NULL, NULL, (s), (s) + (len), 0, TERROR };                                                  \
       p.start < p.end && (((p.token = tcl_next(p.start, p.end - p.start, &p.from, &p.to, &p.q)) != TERROR) || (skiperr)); \
       p.start = p.to)

typedef char tcl_data;

#ifndef TCL_DISABLE_MALLOC

#define TCL_FREE(m) free(m)
#define TCL_MALLOC(m) malloc(m)
#define TCL_REALLOC(m, n) realloc(m, n)

#endif

const char* tcl_string(tcl_data* v);
int tcl_int(tcl_data* v);
int tcl_length(tcl_data* v);

/* void tcl_free(tcl_data* v); */

int tcl_next(const char* s, size_t n, const char** from, const char** to,
    int* q);

tcl_data* tcl_append_string(tcl_data* v, const char* s, size_t len);
tcl_data* tcl_append(tcl_data* v, tcl_data* tail);
tcl_data* tcl_alloc(const char* s, size_t len);
tcl_data* tcl_dup(tcl_data* v);
tcl_data* tcl_list_alloc();
void tcl_list_free(tcl_data* v);
tcl_data* tcl_list_at(tcl_data* v, int index);
tcl_data* tcl_list_append(tcl_data* v, tcl_data* tail);

typedef int (*tcl_cmd_fn_t)(struct tcl*, tcl_data*, void*);

struct tcl_cmd {
  tcl_data* name;
  int arity;
  tcl_cmd_fn_t fn;
  void* arg;
  struct tcl_cmd* next;
};

typedef tcl_atom_t uint64_t;

struct tcl_var {
  tcl_atom_t name;
  tcl_data* value;
  struct tcl_var* next;
};

struct tcl_env {
  tcl_var_t *vars;
  struct tcl_env* parent;
};

struct tcl {
  struct tcl_env* env;
  struct tcl_cmd* cmds;
  tcl_data* result;
};

struct tcl_env* tcl_env_alloc(struct tcl_env* parent);
struct tcl_var* tcl_env_var(struct tcl_env* env, tcl_data* name);
struct tcl_env* tcl_env_free(struct tcl_env* env);
tcl_data* tcl_var(struct tcl* tcl, tcl_data* name, tcl_data* v);
int tcl_result(struct tcl* tcl, int flow, tcl_data* result);
int tcl_subst(struct tcl* tcl, const char* s, size_t len);
int tcl_eval(struct tcl* tcl, const char* s, size_t len);
void tcl_register(struct tcl* tcl, const char* name, tcl_cmd_fn_t fn, int arity,
    void* arg);
void tcl_init(struct tcl* tcl);
void tcl_destroy(struct tcl* tcl);

#ifdef __cplusplus
}
#endif
#endif
