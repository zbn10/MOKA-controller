#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "repl.h" 

appcontext_repl_t &appctxRepl() {
  static appcontext_repl_t ctx = {
    Repl(),
  };
  return ctx;
}

int Repl::addStream(Stream *s) {
  if(s == nullptr || s_count >= 5 ) return -1;

  streams[s_count++] = s;
  this->s = s;
  return 1;
}

int Repl::setStream(Stream *s) {
  if(s == nullptr) return -1;

  this->s = s;
  return 1;
}

int Repl::setCmdset(Cmdset *cs, unsigned int n) {
  if(cmdset) return -1;

  cmdset = cs;
  cmdset_count = n;
  return n;
}

void Repl::help(int ac, const char **av) {
  s->print("###\n");
  s->print("# Firmware Version: ");
  s->print(MF_VERSION_STR);
  s->print(", Board Version: ");
  s->print(MB_VERSION_STR);
  s->print("\n");
  s->print("\n# help\n# show this help.\n");

  if(ac == 0) {
    s->print("rp interval(ms) repeat(>=0), rp c(lear)\n");

    int n = 0;
    while(n < cmdset_count) {
      s->print(cmdset[n].cmd);
      s->print(" : ");
      s->print(cmdset[n].descr);
      s->print("\n");
      n++;
    }
  } else {
    int n = 0;
    while(n < cmdset_count) {
      if(strcmp(av[0], cmdset[n].cmd) == 0) break;
      n++;
    }
    if(n == cmdset_count) {
      s->print("# Unknown command ");
      s->print(av[0]);
      s->print("\nERR\n");
      return;
    }
    s->print(cmdset[n].syntax);
    s->print("\n");
  }
  s->print("###\nOK\n");
}

int Repl::eval(Stream *s, uint8_t *cmdbuf, size_t buflen) {
  Stream *s_backup = this->s;
  this->s = s;

  parser.resetCmd();
  parser.setBuf2CmdStr(cmdbuf, buflen);
  int r = evalInternal();
  this->s = s_backup;

  return(r);
}

int Repl::eval() {
  char *reserved_cmd = pickReserved();
  if(reserved_cmd != nullptr) {
    parser.resetCmd();
    parser.setBuf2CmdStr((uint8_t *)reserved_cmd, strlen(reserved_cmd));
    return(evalInternal());
  }

  if(!s->available()) return 0;

  delay(100);
  parser.resetCmd();
  while(s->available()) {
    char c = s->read();
    if(c == '\a') continue;
    if(c == '\n') break;
    parser.addChar2Cmd(c);
  }
  while(s->available()) { s->read(); }
  char backup_cmdstr[REPL_STRMAX+1];
  strncpy(backup_cmdstr, parser.getCmdStr(), REPL_STRMAX);
  backup_cmdstr[REPL_STRMAX] = '\0';

  int r = evalInternal();
  if(r != -2 && reserve_notice) {
    if(r >= 0) {
      reserve(backup_cmdstr, reserve_notice_delay, reserve_notice_repeat);
    }
    reserve_notice = false;
  }
  return(r);
}

int Repl::exec(const char *cmd) {
  if(cmd == nullptr || cmd[0] == '\0') return 0;

  parser.resetCmd();
  parser.setBuf2CmdStr((uint8_t *)cmd, strlen(cmd));
  return(evalInternal());
}

int Repl::evalInternal() {
  int args = parser.parseCmdStr();
  if(args == 0) return 0;
  const char **argv = parser.getArgv();

  // Repeat command
  if(parser.argvCmp(0, "rp") == 0) {
    if(args < 2) {
      return -1;
    }
    if(*argv[1] == 'c') {
      reserved_count = 0;
      reserved_idx = 0;
      s->print("# cleared reserved commands\n\n");
      return 1;
    } else if(args < 3) {
      s->print("# invalid repeat command, rp <interval> <repeat>\n\n");
      return -1;
    }
    reserve_notice = true;
    reserve_notice_delay = strtol(argv[1], nullptr, 0);
    reserve_notice_repeat = strtol(argv[2], nullptr, 0);
    s->print("# set repeat notice: interval ");
    s->print(reserve_notice_delay);
    s->print(" ms, repeat ");
    s->print(reserve_notice_repeat);
    s->print("\n\n");
    return -2;
  }

  for(int i = 0; i < cmdset_count; i++) {
    if(parser.argvCmp(0, cmdset[i].cmd) == 0) {
      if(cmdset[i].func == nullptr) return -1;

#if defined(DEBUG)
      if(ctxsys.kb_ctrl.isDebugMode()) {
        LOG_DEBUG("cmd[0] = ", cmdset[i].cmd[0], "args = ", args);
      }
#endif
      int r = cmdset[i].func(s, args-1, &(argv[1]));
      s->print("### ");
      s->print(cmdset[i].cmd);
      int i = 1;
      while(i < args) {
        s->print(" ");
        s->print(parser.getArgv(i++));
      }
      s->print("\n");
      s->print("### result = ");
      s->print(r);
      if(r < 0) s->print("\nERR\n");
      else s->print("\nOK\n");
      return r;
    }
  }
  if(parser.argvCmp(0, "help") == 0) {
    help(args-1, &(argv[1]));
    return 1;
  }
  s->print("# Unknown command ");
  s->print(parser.getArgv(0));
  s->print("\nERR\n");
  return -1;
}

bool Repl::reserve(const char *cmd, unsigned long delay_ms, uint8_t repeat) {
  if(reserved_count >= REPL_RESERVED_MAX || delay_ms > REPL_RESERVED_DELAY_MAX || cmd == nullptr) return false;

  uint8_t p = (reserved_idx + reserved_count) % REPL_RESERVED_MAX;
  strncpy(reserved_buf[p], cmd, REPL_STRMAX-1);
  reserved_buf[p][REPL_STRMAX-1] = '\0';
  reserved_time[p] = millis() + delay_ms;
  reserved_delay[p] = delay_ms;
  reserved_repeat[p] = repeat;
  reserved_count++;
#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("reserved:", cmd, ", -> idx:", reserved_idx, ", count:", reserved_count);
    }
#endif

  return true;
}

char *Repl::pickReserved() {
  if(reserved_count == 0) return nullptr;

  char *cmd = nullptr;
  unsigned long now = millis();

  unsigned long diff_ms = reserved_time[reserved_idx] - now;
  if(diff_ms > REPL_RESERVED_DELAY_MAX) { // overflow
    cmd = reserved_buf[reserved_idx];

    if(reserved_repeat[reserved_idx] == 1) {
      reserved_idx = (reserved_idx + 1) % REPL_RESERVED_MAX;
      reserved_count--;
    } else {
      reserved_time[reserved_idx] = now + reserved_delay[reserved_idx];
      if(reserved_repeat[reserved_idx] > 0) reserved_repeat[reserved_idx]--;
    }

#if defined(DEBUG)
    if(ctxsys.kb_ctrl.isDebugMode()) {
      LOG_DEBUG("reserved:", cmd, ", -> idx:", reserved_idx, ", count:", reserved_count);
    }
#endif
  }

  return cmd;
}
