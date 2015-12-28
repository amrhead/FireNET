/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 14.08.2014   14:07 : Created by AfroStalin(chernecoff)
- 04.06.2015   11:37 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#include "StdAfx.h"
#include "Settings.h"

#include <iostream>
#include <fstream>

#pragma warning(disable : 4018)
#pragma warning(disable : 4267)

/////////////////////// Default params ////////////////////////////

char* defParams = "[Server]\n"
	              "ip=127.0.0.1\n"
				  "port=64087\n"
				  "max_players=16\n"
				  "max_gameservers=1\n"
				  "use_xml=1\n"
				  "block_dual_servers=1\n"
				  "block_dual_players=1\n"
				  "securityKey=PLEASE_CHANGE_YOU_MASTER_SERVER_SUPER_STRONG_ENCRYPTION_KEY_HERE";

////////////////////////////////////////////////////////////////////

char valueBuffer[256];

int get_parm_offset(char * str,int nom)
{ 
	while((str[nom]!=',')&&(str[nom]!=';')&&(str[nom]!='\0')) nom++;
	if((str[nom]==';')||(str[nom]=='\0')) return 0;
	return(++nom);
}

int conv_string(char *str,int *save_buffer)
{	
	int nom=0;
	int count=0;

	if(sscanf(str,"%d,",&save_buffer[count])>0) count++;

	while ((nom=get_parm_offset(str,nom))!=0)
		sscanf(str+nom,"%d,",&save_buffer[count++]);

	return count;
}

int read_sec_parm(char *filename,char *sect_name,char *parm_name,char *save_buffer,size_t save_size)
{
	FILE *cfg_file;
	char Order[4000];
	char buf[4000];
	int num_char=0;

	memset(Order,' ',sizeof(Order));

	cfg_file=fopen(filename,"r");

	if(cfg_file==NULL) 
	{ 
		Log(LOG_WARNING, "server.cfg not found! Using default settings...");

		// Create default settings file
		cfg_file=fopen(filename,"a");
		fprintf(cfg_file,defParams);
		fclose(cfg_file);
	}

	cfg_file=fopen(filename,"r");

	while(fgets(buf,sizeof(buf),cfg_file)!=NULL)
	{	
		int i=0;
		char ch=buf[i];

		while((ch==' ')||(ch=='\t')) ch=buf[++i];


		if ((ch=='#')||(ch==';')||(ch=='\n')) continue;

		if(sscanf(buf+i,"[%s]",Order)!=0)	continue;

		if (strncmp(sect_name,Order,strlen(sect_name))!=0)	continue;

		if (strncmp(parm_name,buf+i,strlen(parm_name))!=0)	continue;

		int j=i;
		ch=buf[j];

		while((ch!='\t')&&(ch!=';')&&(ch!='#')&&(ch!='\n')) ch=buf[++j];
		num_char=j-(i+strlen(parm_name)+sizeof('='));
		if(num_char>=save_size) {num_char=-1;break;}
		strncpy(save_buffer,&buf[i+strlen(parm_name)+sizeof('=')],num_char);
		save_buffer[num_char]=0;

		break;
	}

	fclose(cfg_file);
	return num_char;
}

const char* CSettings::GetConfigValue(char* sectionName, char* valueName)
{
	read_sec_parm("server.cfg",sectionName,valueName,valueBuffer,256);
	return valueBuffer;
}