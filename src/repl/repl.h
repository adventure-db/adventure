#ifndef ADV_REPL_H
#define ADV_REPL_H

void repl_add_cmd(char *str, int (*fn)(char *cmd));
int repl(const char *prompt);

#endif
