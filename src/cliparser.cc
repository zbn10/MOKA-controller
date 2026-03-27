#include <Arduino.h>
#include "debuglog.h"
#include <DebugLog.h>
#include "controller.h"
#include "sysutils.h"
#include "i2c_control.h"
#include "cliparser.h"

//appcontext_sys_t &ctxsys = appctxSys();

int CliParser::init(unsigned int acmax, unsigned int clmax) {
  if(cmdstr) return(-1);

  argv = (char **)malloc(sizeof(char *) * acmax + clmax + 1);
  if(!argv) {
    return(-1);
  }

  argcmax = acmax;
  cmdstrlenmax = clmax;
  cmdstr = ((char *)argv + sizeof(char *) * argcmax);
  return(1);
}

int CliParser::resetCmd(void) {
  if(!cmdstr) return(-1);

  argc = 0;
  cmdstrlen = 0;
  return(1);
}

int CliParser::setBuf2CmdStr(uint8_t *buf, size_t buflen) {
  if(buf == nullptr || buflen <= 0) return(-1);

  cmdstrlen = (buflen > cmdstrlenmax ? cmdstrlenmax : buflen);
  memcpy(cmdstr, buf, cmdstrlen);
  cmdstr[cmdstrlen] = '\0';
#if defined(DEBUG)
  if(ctxsys.kb_ctrl.isDebugMode()) {
    LOG_DEBUG("len = ", cmdstrlen, ", cmdstr = \"", cmdstr, "\"");
  }
#endif
  return(cmdstrlen);
}

int CliParser::addChar2Cmd(char c) {
  if(!cmdstr || cmdstrlen >= cmdstrlenmax) return(-1);

  cmdstr[cmdstrlen++] = c;
  cmdstr[cmdstrlen] = '\0';
  return(1);
}

int CliParser::parseCmdStr() {
  if(!cmdstr) return(-1);

  int i = 0;
  int firstletter = 1;
  argc = 0;

  while(i < cmdstrlen) {
    if(cmdstr[i] == ' ') {
      cmdstr[i++] = '\0';
      firstletter = 1;
      continue;
    }

    if(firstletter) {
      firstletter = 0;
      if(argc < argcmax) {
        argv[argc++] = cmdstr+i;
      } else {
        return(argc);
      }
    }

    i++;
  }

  return(argc);
}

int CliParser::getArgc(void) {
  return(argc);
}

const char **CliParser::getArgv(void) {
  return((const char **)argv);
}

char *CliParser::getArgv(unsigned int n) {
  if(n < 0 || n > argc) return(nullptr);
  return(argv[n]);
}

int CliParser::getArgv(unsigned int n, char *buf, unsigned int buflen) {
  // return <0 = err, 0 = lack of buflen, >0 = success
  if(n > argc || buf == nullptr) return(-1);

  unsigned int len = strlen(argv[n]);
  if(buflen >= len + 1) {
    memcpy(buf, argv[n], len+1);
    return(1);
  } else {
    memcpy(buf, argv[n], buflen - 1);
    buf[buflen] = '\0';
    return(0);
  }
}

int CliParser::argvCmp(unsigned int n, const char *str) {
  if(n > argc) return(-1);
  return(strcmp(argv[n], str));
}
