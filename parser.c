#include "common.h"

extern gchar *parse(gchar *str)
{
	gchar *cmd,*param1,*param2;
	int len;
	len = strchr(str, ' ')-str;
	strncpy(cmd, str, len);
	cmd[len]='\0';
	g_print("%s",cmd);
}
