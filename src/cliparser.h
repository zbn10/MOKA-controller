#pragma once

class CliParser {
  private:
    char *cmdstr;
    char **argv;
    unsigned int argcmax;
    unsigned int cmdstrlenmax;
    unsigned int argc;
    unsigned int cmdstrlen;

  public:
    CliParser() : cmdstr(nullptr), argv(nullptr), argcmax(0), cmdstrlenmax(0), argc(0), cmdstrlen(0) {}
    int init(unsigned int, unsigned int);
    int resetCmd();
    int setBuf2CmdStr(uint8_t *, size_t);
    int addChar2Cmd(char);
    const char *getCmdStr() { return (const char *)cmdstr; }
    int parseCmdStr();
    int getArgc(void);
    const char **getArgv(void);
    char *getArgv(unsigned int);
    int getArgv(unsigned int, char*, unsigned int);
    int argvCmp(unsigned int, const char *);
    //void debugPrint(Adafruit_USBD_CDC &);
};
