#include <stdlib.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tcl.h"

#if 0
#define DBG printf
#else
#define DBG(...)
#endif

#ifndef MAX_VAR_LENGTH
#define MAX_VAR_LENGTH 256
#endif

struct tcl;

typedef stream_t tcl_data;

/* ------------------------------------------------------- */
/* ------------------------------------------------------- */

static inline void* tcl_malloc(tcl_size_t n) { return TCL_MALLOC(n); }
static inline void tcl_free(void* v) { TCL_FREE(v); }
static inline void* tcl_realloc(void* v, int n) { return TCL_REALLOC(v, n); }

tcl_data* tcl_data_alloc(size_t len)
{
  tcl_data stream = tcl_malloc(sizeof(tcl_data));
  stream->buf = tcl_malloc(len);
  stream->len = len;
  stream->pos = 0;
  stream->max = 0;
}

tcl_data* tcl_data_alloc_nil()
{
  tcl_data *buf = tcl_data_alloc(1);
  msgpck_write_nil(buf);
  return buf;
}

tcl_data* tcl_data_dup(tcl_data *buf)
{
  ssize_t len = buf->len;
  tcl_data *copy = tcl_data_alloc(len);
  memcpy(copy->buf, buf->buf, len);
  return copy;
}

tcl_data tcl_data_clone(tcl_data *buf)
{
  tcl_data clone;

  clone->len = buf->len;
  clone->max = buf->max;
  clone->pos = buf->pos;

  return clone;
}


void tcl_data_free(tcl_data *buf) {
  free(buf->buf);
  free(buf);
}

/* ------------------------------------------------------- */
/* ------------------------------------------------------- */
/* ------------------------------------------------------- */

int tcl_list_length(tcl_data* v)
{
  ssize_t array_size;

  tcl_data clone = tcl_data_clone(v);

  if (msgpck_read_array_size(&clone, &array_size))
    return array_size;
  else
    return -1;
}

tcl_token_t tcl_next(tcl_data* buf)
{
  msgpck_type_t type;
  bool status;

  status = msgpck_skip_next(buf, &type);

  if (!status)
    return TERROR;

  switch (type) {

  case msgpck_empty:
    return TCMD;

  case msgpck_nil:
  case msgpck_bool:
  case msgpck_bin:
  case msgpck_string:
  case msgpck_sint:
  case msgpck_uint:
  case msgpck_array:
  case msgpck_map:
    return TWORD;

  default:
    return TERROR;
  }

  return TERROR;
}

int tcl_result(struct tcl* tcl, int flow, tcl_data* result)
{
  DBG("tcl_result %.*buf, flow=%d\n", tcl_length(result), tcl_string(result), flow);
  tcl_free(tcl->result);
  tcl->result = result;
  return flow;
}


tcl_data* tcl_subst(tcl_data buf, ssize_t len)
{
  msgpck_type_t ntype = msgpck_what_next(buf);
  uint32_t n;
  bool r = true;

  switch (ntype ) {
  case msgpck_map:
    r &= msgpck_read_map_size(buf, &n);
    r &= msgpck_read_nil(buf);
    r &= tcl_next(buf) != TERROR;
    return tcl_result(tcl, FNORMAL, tcl_alloc(buf + 1, len - 2));
  }

  tcl_data *copy = tcl_alloc_stream(lstlen);
  return copy;
}

tcl_data* tcl_list_at(tcl_data* data_list, int index)
{
  bool status = true;
  uint32_t array_size;
  msgpck_type_t mtype;

  tcl_data buf = tcl_data_clone(data_list);
  status = msgpck_read_array_size(buf, &array_size);

  if (!status || array_size <= index)
    return NULL;

  uint8_t start_pos = buf->pos;

  int i = 0;
  while ( (mtype = msgpck_what_next(buf)) != msgpck_empty) {
    status = msgpck_skip_next(buf, &mtype);

    if (!status)
      return NULL;

    if (index == i++) {
      if (ttype == msgpck_map) {
        msgpck_read_nil(buf);
        
      }
      return tcl_data_slice(v);
    }
  }


  return NULL;
}

char* tcl_list_append(tcl_data *v, char* tail)
{
}

/* ----------------------------- */
/* ----------------------------- */
/* ----------------------------- */
/* ----------------------------- */

tcl_env_t* tcl_env_alloc(tcl_env_t* parent)
{
  tcl_env_t* env = tcl_malloc(sizeof(struct tcl_env));

  env->vars = NULL;
  env->parent = parent;

  return env;
}

tcl_var_t* tcl_env_var_alloc(tcl_env_t* env, tcl_atom_t name)
{
  tcl_var_t* var = tcl_malloc(sizeof(tcl_var_t));

  var->name = name;
  var->next = env->vars;

  var->value = tcl_data_alloc_nil();

  msgpck_write_nil(var->value);

  // update var list
  env->vars = var;

  return var;
}

tcl_env_t* tcl_env_free(tcl_env_t* env)
{
  tcl_env_t* parent = env->parent;
  while (env->vars) {
    tcl_var_t* var = env->vars;
    env->vars = env->vars->next;
    tcl_free(var->name);
    tcl_free(var->value);
    tcl_free(var);
  }
  tcl_free(env);
  return parent;
}

char* tcl_var(struct tcl* tcl, char* name, tcl_data* v)
{
  DBG("var(%buf := %.*buf)\n", tcl_string(name), tcl_length(v), tcl_string(v));
  tcl_var_t* var;
  for (var = tcl->env->vars; var != NULL; var = var->next) {
    if (strcmp(var->name, tcl_string(name)) == 0) {
      break;
    }
  }
  if (var == NULL) {
    var = tcl_env_var(tcl->env, name);
  }
  if (v != NULL) {
    tcl_free(var->value);
    var->value = tcl_dup(v);
    tcl_free(v);
  }
  return var->value;
}

int tcl_result(struct tcl* tcl, int flow, char* result)
{
  DBG("tcl_result %.*buf, flow=%d\n", tcl_length(result), tcl_string(result),
      flow);
  tcl_free(tcl->result);
  tcl->result = result;
  return flow;
}

int tcl_subst(struct tcl* tcl, tcl_data *buf)
{
  DBG("subst(%.*buf)\n", (int)len, buf);
  if (len == 0) {
    return tcl_result(tcl, FNORMAL, tcl_alloc("", 0));
  }
  switch (buf[0]) {
  case '{':
    if (len <= 1) {
      return tcl_result(tcl, FERROR, tcl_alloc("", 0));
    }
    return tcl_result(tcl, FNORMAL, tcl_alloc(buf + 1, len - 2));
  case '$': {
    if (len >= MAX_VAR_LENGTH) {
      return tcl_result(tcl, FERROR, tcl_alloc("", 0));
    }
    char buf[5 + MAX_VAR_LENGTH] = "set ";
    strncat(buf, buf + 1, len - 1);
    return tcl_eval(tcl, buf, strlen(buf) + 1);
  }
  case '[': {
    char* expr = tcl_alloc(buf + 1, len - 2);
    int r = tcl_eval(tcl, tcl_string(expr), tcl_length(expr) + 1);
    tcl_free(expr);
    return r;
  }
  default:
    return tcl_result(tcl, FNORMAL, tcl_alloc(buf, len));
  }
}

int tcl_eval(struct tcl* tcl, tcl_data *buf)
{
  DBG("eval(%.*buf)->\n", (int)len, buf);
  char* list = tcl_list_alloc();
  char* cur = NULL;
  tcl_each(buf, len, 1)
  {
    DBG("tcl_next %d %.*buf\n", p.token, (int)(p.to - p.from), p.from);
    switch (p.token) {
    case TERROR:
      DBG("eval: FERROR, lexer error\n");
      return tcl_result(tcl, FERROR, tcl_alloc("", 0));
    case TWORD:
      DBG("token %.*buf, length=%d, cur=%p (3.1.1)\n", (int)(p.to - p.from),
          p.from, (int)(p.to - p.from), cur);
      if (cur != NULL) {
        tcl_subst(tcl, p.from, p.to - p.from);
        char* part = tcl_dup(tcl->result);
        cur = tcl_append(cur, part);
      } else {
        tcl_subst(tcl, p.from, p.to - p.from);
        cur = tcl_dup(tcl->result);
      }
      list = tcl_list_append(list, cur);
      tcl_free(cur);
      cur = NULL;
      break;
    case TPART:
      tcl_subst(tcl, p.from, p.to - p.from);
      char* part = tcl_dup(tcl->result);
      cur = tcl_append(cur, part);
      break;
    case TCMD:
      if (tcl_list_length(list) == 0) {
        tcl_result(tcl, FNORMAL, tcl_alloc("", 0));
      } else {
        char* cmdname = tcl_list_at(list, 0);
        struct tcl_cmd* cmd = NULL;
        int r = FERROR;
        for (cmd = tcl->cmds; cmd != NULL; cmd = cmd->next) {
          char *c1 = tcl_string(cmdname);
          char *c2 =tcl_string(cmd->name);

          if (tcl_strcmp(c1, c2) == 0) {
            if (cmd->arity == 0 || cmd->arity == tcl_list_length(list)) {
              r = cmd->fn(tcl, list, cmd->arg);
              break;
            }
          }
        }
        tcl_free(cmdname);
        if (cmd == NULL || r != FNORMAL) {
          tcl_list_free(list);
          return r;
        }
      }
      tcl_list_free(list);
      list = tcl_list_alloc();
      break;
    }
  }
  tcl_list_free(list);
  return FNORMAL;
}

/* --------------------------------- */
/* --------------------------------- */
/* --------------------------------- */
/* --------------------------------- */
/* --------------------------------- */
void tcl_register(struct tcl* tcl, const char* name, tcl_cmd_fn_t fn, int arity,
    void* arg)
{
  struct tcl_cmd* cmd = tcl_malloc(sizeof(struct tcl_cmd));
  cmd->name = tcl_alloc(name, strlen(name));
  cmd->fn = fn;
  cmd->arg = arg;
  cmd->arity = arity;
  cmd->next = tcl->cmds;
  tcl->cmds = cmd;
}

static int tcl_cmd_set(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  char* var = tcl_list_at(args, 1);
  char* val = tcl_list_at(args, 2);
  int r = tcl_result(tcl, FNORMAL, tcl_dup(tcl_var(tcl, var, val)));
  tcl_free(var);
  return r;
}

static int tcl_cmd_subst(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  char* buf = tcl_list_at(args, 1);
  int r = tcl_subst(tcl, tcl_string(buf), tcl_length(buf));
  tcl_free(buf);
  return r;
}

static int tcl_user_proc(struct tcl* tcl, char* args, void* arg)
{
  char* code = (char*)arg;
  char* params = tcl_list_at(code, 2);
  char* body = tcl_list_at(code, 3);
  tcl->env = tcl_env_alloc(tcl->env);
  for (int i = 0; i < tcl_list_length(params); i++) {
    char* param = tcl_list_at(params, i);
    char* v = tcl_list_at(args, i + 1);
    tcl_var(tcl, param, v);
    tcl_free(param);
  }
  int r = tcl_eval(tcl, tcl_string(body), tcl_length(body) + 1);
  tcl->env = tcl_env_free(tcl->env);
  tcl_free(params);
  tcl_free(body);
  /* return FNORMAL; */
  return r;
}

static int tcl_cmd_proc(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  char* name = tcl_list_at(args, 1);
  tcl_register(tcl, tcl_string(name), tcl_user_proc, 0, tcl_dup(args));
  tcl_free(name);
  return tcl_result(tcl, FNORMAL, tcl_alloc("", 0));
}

static int tcl_cmd_if(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  int i = 1;
  int n = tcl_list_length(args);
  int r = FNORMAL;
  while (i < n) {
    char* cond = tcl_list_at(args, i);
    char* branch = NULL;
    if (i + 1 < n) {
      branch = tcl_list_at(args, i + 1);
    }
    r = tcl_eval(tcl, tcl_string(cond), tcl_length(cond) + 1);
    tcl_free(cond);
    if (r != FNORMAL) {
      tcl_free(branch);
      break;
    }
    if (tcl_int(tcl->result)) {
      r = tcl_eval(tcl, tcl_string(branch), tcl_length(branch) + 1);
      tcl_free(branch);
      break;
    }
    i = i + 2;
    tcl_free(branch);
  }
  return r;
}

static int tcl_cmd_flow(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  int r = FERROR;
  char* flowval = tcl_list_at(args, 0);
  const char* flow = tcl_string(flowval);
  if (strcmp(flow, "break") == 0) {
    r = FBREAK;
  } else if (strcmp(flow, "continue") == 0) {
    r = FAGAIN;
  } else if (strcmp(flow, "return") == 0) {
    r = tcl_result(tcl, FRETURN, tcl_list_at(args, 1));
  }
  tcl_free(flowval);
  return r;
}

static int tcl_cmd_while(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  char* cond = tcl_list_at(args, 1);
  char* loop = tcl_list_at(args, 2);
  int r;
  for (;;) {
    r = tcl_eval(tcl, tcl_string(cond), tcl_length(cond) + 1);
    if (r != FNORMAL) {
      tcl_free(cond);
      tcl_free(loop);
      return r;
    }
    if (!tcl_int(tcl->result)) {
      tcl_free(cond);
      tcl_free(loop);
      return FNORMAL;
    }
    int r = tcl_eval(tcl, tcl_string(loop), tcl_length(loop) + 1);
    switch (r) {
    case FBREAK:
      tcl_free(cond);
      tcl_free(loop);
      return FNORMAL;
    case FRETURN:
      tcl_free(cond);
      tcl_free(loop);
      return FRETURN;
    case FAGAIN:
      continue;
    case FERROR:
      tcl_free(cond);
      tcl_free(loop);
      return FERROR;
    }
  }
  return FERROR;
}

#ifndef TCL_DISABLE_MATH
static int tcl_cmd_math(struct tcl* tcl, char* args, void* arg)
{
  (void)arg;
  char buf[64];
  char* opval = tcl_list_at(args, 0);
  char* aval = tcl_list_at(args, 1);
  char* bval = tcl_list_at(args, 2);
  const char* op = tcl_string(opval);
  int a = tcl_int(aval);
  int b = tcl_int(bval);
  int c = 0;
  if (op[0] == '+') {
    c = a + b;
  } else if (op[0] == '-') {
    c = a - b;
  } else if (op[0] == '*') {
    c = a * b;
  } else if (op[0] == '/') {
    c = a / b;
  } else if (op[0] == '>' && op[1] == '\0') {
    c = a > b;
  } else if (op[0] == '>' && op[1] == '=') {
    c = a >= b;
  } else if (op[0] == '<' && op[1] == '\0') {
    c = a < b;
  } else if (op[0] == '<' && op[1] == '=') {
    c = a <= b;
  } else if (op[0] == '=' && op[1] == '=') {
    c = a == b;
  } else if (op[0] == '!' && op[1] == '=') {
    c = a != b;
  }

  char* p = buf + sizeof(buf) - 1;
  char neg = (c < 0);
  *p-- = 0;
  if (neg) {
    c = -c;
  }
  do {
    *p-- = '0' + (c % 10);
    c = c / 10;
  } while (c > 0);
  if (neg) {
    *p-- = '-';
  }
  p++;

  tcl_free(opval);
  tcl_free(aval);
  tcl_free(bval);
  return tcl_result(tcl, FNORMAL, tcl_alloc(p, strlen(p)));
}
#endif

void tcl_init(struct tcl* tcl)
{
  tcl->env = tcl_env_alloc(NULL);
  tcl->result = tcl_alloc("", 0);
  tcl->cmds = NULL;
  tcl_register(tcl, "set", tcl_cmd_set, 0, NULL);
  tcl_register(tcl, "subst", tcl_cmd_subst, 2, NULL);
#ifndef TCL_DISABLE_PUTS
  tcl_register(tcl, "puts", tcl_cmd_puts, 2, NULL);
#endif
  tcl_register(tcl, "proc", tcl_cmd_proc, 4, NULL);
  tcl_register(tcl, "if", tcl_cmd_if, 0, NULL);
  tcl_register(tcl, "while", tcl_cmd_while, 3, NULL);
  tcl_register(tcl, "return", tcl_cmd_flow, 0, NULL);
  tcl_register(tcl, "break", tcl_cmd_flow, 1, NULL);
  tcl_register(tcl, "continue", tcl_cmd_flow, 1, NULL);
#ifndef TCL_DISABLE_MATH
  char* math[] = { "+", "-", "*", "/", ">", ">=", "<", "<=", "==", "!=" };
  for (unsigned int i = 0; i < (sizeof(math) / sizeof(math[0])); i++) {
    tcl_register(tcl, math[i], tcl_cmd_math, 3, NULL);
  }
#endif
}

void tcl_destroy(struct tcl* tcl)
{
  while (tcl->env) {
    tcl->env = tcl_env_free(tcl->env);
  }
  while (tcl->cmds) {
    struct tcl_cmd* cmd = tcl->cmds;
    tcl->cmds = tcl->cmds->next;
    tcl_free(cmd->name);
    tcl_free(cmd->arg);
    tcl_free(cmd);
  }
  tcl_free(tcl->result);
}
