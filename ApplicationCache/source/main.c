#include "ps4.h"
#include "patch.h"
#include "cache.h"

char ApplicationCache[] = "/user/system/webkit/webbrowser/appcache/ApplicationCache.db";
int Configuration = 1337;

void init(struct thread *Thread)
{
	initKernel();
	initLibc();
	syscall(11, KernelPatch, Thread);
	initSysUtil();
}

void GetUserId(char* Buffer)
{
	DIR *Directory = opendir("/user/home/");
	
	if (Directory)
	{
		struct dirent *DirectoryEntity;
		
		while ((DirectoryEntity = readdir(Directory)) != NULL)
		{
			if (strstr(DirectoryEntity->d_name, ".") != NULL)
			{
				continue;
			}
			
			memcpy(Buffer, DirectoryEntity->d_name, strlen(DirectoryEntity->d_name));
			break;
		}
		
		closedir(Directory);
	}
}

void SystemMessage(char* Input)
{
 	sceSysUtilSendSystemNotificationWithText(0xDE, Input);
}

int WriteFile(char* File, unsigned char* Buffer, unsigned int Length)
{
	char Directory[256];
	memcpy(Directory, File, strlen(File));
	
	if (Directory[strlen(Directory) - 1] == '/')
	{
		Directory[strlen(Directory) - 1] = 0;
	}
	
	for (char* Index = Directory + 1; *Index; Index++)
	{
		if (*Index == '/')
		{
			*Index = 0;
			mkdir(Directory, 0777);
			*Index = '/';
		}
	}
	
	int Handle = open(File, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	
	if (Handle != -1)
	{
		write(Handle, Buffer, Length);
		close(Handle);
	}
	
	return Handle;
}

int _main(struct thread *Thread)
{
	init(Thread);
	
	if ((Configuration == 0) || (Configuration == 1))
	{
		char UserId[16] = "";
	 	GetUserId(UserId);
		
		if (strlen(UserId) > 0)
		{
			char File[256];
			sprintf(File, "/user/home/%s/webbrowser/endhistory.txt", UserId);
			
			rmdir(File);
			unlink(File);
			
			if (Configuration == 1)
			{
				mkdir(File, 0777);
			}
		}
	}
	
	SystemMessage("Attempting to create file...\nPlease wait...");
		
	if (WriteFile(ApplicationCache, ApplicationCache_db, ApplicationCache_db_len) != -1)
	{
		SystemMessage("Done.");
	}
	else
	{
		SystemMessage("ERROR: Unable to create file.");
	}
	
	return 0;
}
