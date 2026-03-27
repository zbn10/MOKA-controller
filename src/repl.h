#pragma once
#include <Arduino.h>
#include "cliparser.h"

#define CMDSET(f) (f), (f##Cmd), (f##Syntax), (f##Descr)
class Cmdset {
public:
  Cmdset(int (*f)(Stream *, uint8_t, const char **),
    const char *cmd, const char *syntax, const char *descr) : func(f), cmd(cmd), syntax(syntax), descr(descr) {}
  int (*func)(Stream *, uint8_t, const char **);
  const char *cmd;
  const char *syntax;
  const char *descr;
};

class Repl {
public:
#define REPL_STRMAX 200
#define REPL_ARGVMAX 20
  Repl() { parser.init(REPL_ARGVMAX, REPL_STRMAX); }
  int setStream(Stream *);
  int addStream(Stream *);
  Stream *getStream() { return s;};
  int setCmdset(Cmdset *, unsigned int);
  int eval();
  int eval(Stream *, uint8_t *, size_t);
  int exec(const char *);

  bool reserve(const char *s, unsigned long delay_ms, uint8_t repeat = 1);

private:
  Stream *s = &Serial, *streams[5];
  int s_count = 0;
  Cmdset *cmdset = nullptr;
  unsigned int cmdset_count = 0;
  CliParser parser;

#define REPL_RESERVED_MAX 10
#define REPL_RESERVED_DELAY_MAX 10000 // ms
  uint8_t reserved_count = 0;
  uint8_t reserved_idx = 0;
  char reserved_buf[REPL_RESERVED_MAX][REPL_STRMAX];
  unsigned long reserved_time[REPL_RESERVED_MAX];
  unsigned long reserved_delay[REPL_RESERVED_MAX];
  uint8_t reserved_repeat[REPL_RESERVED_MAX];
  bool reserve_notice = false;
  unsigned long reserve_notice_delay = 0;
  uint8_t reserve_notice_repeat = 0;

  int evalInternal();
  char *pickReserved();
  void help(int ac, const char **av);
};

typedef struct {
  Repl repl;
} appcontext_repl_t;
appcontext_repl_t &appctxRepl();
inline appcontext_repl_t &ctxrepl = appctxRepl();
