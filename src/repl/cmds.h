#ifndef ADV_REPL_CMDS_H
#define ADV_REPL_CMDS_H

// Informational
int status(char *line);

// Database admin
int create_db(char *line);
int load_json(char *line);

// Easter eggs
int sting(char *line);

#endif
