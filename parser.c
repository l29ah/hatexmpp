#include "common.h"

gchar *parse(gchar *str)
{
	gchar *token;
	token = strtok(str," ");
	while (token) 
	{
		if (strcmp(token,"send")) 
		token = strtok(str," ,.-"); 
	}
}
