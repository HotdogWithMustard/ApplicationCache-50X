#include "ps4.h"
#include "patch.h"

char ApplicationCache[] = "/user/system/webkit/webbrowser/appcache/ApplicationCache.db";

int ApplicationCacheLength = 1701888;
int Configuration = 1337;

char URL[5][128] =
{
	"http://files.kkhost.pl/files/gen/ulO5M_fEM0/applicationcache.db",
	"http://files.kkhost.pl/files/gen/57ihwPGf3_/applicationcache.db",
	"http://files.kkhost.pl/files/gen/V9HSVhHZtm/applicationcache.db",
	"http://files.kkhost.pl/files/gen/KrPhkgPpeJ/applicationcache.db",
	"http://files.kkhost.pl/files/gen/5UvYRqYZsE/applicationcache.db"
};

int (*sceNetPoolCreate)(const char *name, int size, int flags);
int (*sceNetPoolDestroy)(int memid);
int (*sceNetResolverCreate)(const char *name, int memid, int flags);
int (*sceNetResolverDestroy)(int rid);
int (*sceNetResolverStartNtoa)(int rid, const char *hostname, struct in_addr *addr, int timeout, int retry, int flags);

void init(struct thread *Thread)
{
	initKernel();
	initLibc();
	syscall(11, KernelPatch, Thread);
	initNetwork();
	initSysUtil();
	
	RESOLVE(libNetHandle, sceNetPoolCreate);
	RESOLVE(libNetHandle, sceNetPoolDestroy);
	RESOLVE(libNetHandle, sceNetResolverDestroy);
	RESOLVE(libNetHandle, sceNetResolverCreate);
	RESOLVE(libNetHandle, sceNetResolverStartNtoa);
}

struct GetResponse
{
	int Length;
	int Status;
};

struct GetResponse GetRequest(char* URL, char* Buffer, int Length)
{
	struct GetResponse Response = { 0, -1 };
	int Socket = sceNetSocket("NetSocketGetRequest", AF_INET, SOCK_STREAM, 0);
	
	if (Socket)
	{
		char Domain[256] = "";
		char Path[2048] = "/";
		
		strcat(Domain, URL);
		
		for (int Index = 0; Index <= 1; Index++)
		{
			char* SubString = strstr(Domain, Index > 0 ? "/" : "://");
			
			if (SubString)
			{
				if (Index > 0)
				{
					Index = SubString - Domain;
					
					memcpy(Path, Domain + Index, strlen(Domain) - Index);
					Domain[Index] = 0;
				}
				else
				{
					sprintf(Domain, "%s", SubString + 3);
				}
			}
		}
		
		struct sockaddr_in Server;
		
		int NetPoolId = sceNetPoolCreate("NetPoolGetRequest", 16 * 1024, 0);
		int ResolverId = sceNetResolverCreate("NetResolverGetRequest", NetPoolId, 0);
		int ResolverResult = sceNetResolverStartNtoa(ResolverId, Domain, &Server.sin_addr, 2000000, 3, 0);
		
		Response.Status--;
		
		sceNetResolverDestroy(ResolverId);
		sceNetPoolDestroy(NetPoolId);
		
		if ((NetPoolId > -1) && (ResolverId > -1) && (ResolverResult > -1))
		{
			Server.sin_len = sizeof(Server);
			Server.sin_family = AF_INET;
			Server.sin_port = sceNetHtons(((strstr(URL, "s://")) || (strstr(URL, "S://")) ? 443 : 80));
			
			memset(Server.sin_zero, 0, sizeof(Server.sin_zero));
			
			sprintf(Buffer, "GET %s HTTP/1.1\r\n", Path);
			sprintf(Buffer, "%sHost: %s\r\n", Buffer, Domain);
			strcat(Buffer, "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\r\n");
			strcat(Buffer, "Connection: close\r\n\r\n");
			
			Response.Status++;
			
			if (sceNetConnect(Socket, (struct sockaddr*) &Server, sizeof(Server)) > -1)
			{
				if (sceNetSend(Socket, Buffer, strlen(Buffer), 0) > -1)
				{
					int Index = 0;
					int Received = 0;
					
					char Recv[2048];
					memset(Buffer, 0, Length);
					
					while ((Received = sceNetRecv(Socket, Recv, sizeof(Recv), 0)) > 0)
					{
						Index = 0;
						
						if (Response.Length + Received >= Length)
						{
							Response.Status = -3;
							break;
						}
						
						if (Response.Length < 1)
						{
							char RecvTemp[2048];
							memcpy(RecvTemp, Recv, Received);
							
							for (int I = 0; I < Received; I++)
							{
								if ((RecvTemp[I] > 64) && (RecvTemp[I] < 91))
								{
									RecvTemp[I] += 32;
								}
							}
							
							char* SubStr = strstr(RecvTemp, "location: ");
							
							if (SubStr)
							{
								Index = (SubStr - RecvTemp) + 10;
								SubStr = strstr(Recv + Index, "\r\n");
								
								if (!SubStr)
								{
									break;
								}
								
								char Loc[2048];
								memcpy(Loc, Recv + Index, (SubStr - Recv) - Index);
								
								sceNetSocketClose(Socket);
								return GetRequest(Loc, Buffer, Length);
							}
							
							SubStr = strstr(Recv, "\r\n\r\n");
							
							if (SubStr)
							{
								Index = (SubStr - Recv) + 4;
							}
							
							Response.Status = 0;
							
							for (int I = 9; I < 12; ++I)
							{
								Response.Status = Response.Status*10+Recv[I]-'0';
							}
						}
						
						memcpy(Buffer + Response.Length, Recv + Index, Received - Index);
						Response.Length += Received - Index;
					}
				}
			}
		}
		
		sceNetSocketClose(Socket);
	}
	
	return Response;
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

int WriteFile(char* File, char* Buffer, int Length)
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
	
	char* Buffer;
	int Length = 5242880;
	
	Buffer = (char*) malloc(Length);
	
	if (Buffer)
	{
		struct GetResponse Response = { 0, 0 };
		SystemMessage("Attempting to download file... Please wait...");
		
		for (int Index = 0; Index < sizeof(URL) / sizeof(URL[0]); Index++)
		{
			Response = GetRequest(URL[Index], Buffer, Length);
			
			if (Response.Status == 200)
			{
				if (Response.Length != ApplicationCacheLength)
				{
					Response.Status = -4;
					continue;
				}
				
				if (WriteFile(ApplicationCache, Buffer, Response.Length) == -1)
				{
					Response.Status = -5;
				}
				
				break;
			}
		}
		
		switch (Response.Status)
		{
			case -1:
			{
				SystemMessage("ERROR: Unable to create socket.");
				break;
			}
			case -2:
			{
				SystemMessage("ERROR: Unable to perform DNS lookup.");
				break;
			}
			case -3:
			{
				SystemMessage("ERROR: Response exceeds buffer length.");
				break;
			}
			case -4:
			{
				SystemMessage("ERROR: File size mismatch.");
				break;
			}
			case -5:
			{
				SystemMessage("ERROR: Unable to create file.");
				break;
			}
			case 200:
			{
				SystemMessage("Done.");
				break;
			}
			default:
			{
				char Log[128];
				sprintf(Log, "ERROR: Received invalid status code. (%d)", Response.Status);
				SystemMessage(Log);
			}
		}
		
		free(Buffer);
	}
	else
	{
		SystemMessage("ERROR: Failed to allocate memory.");
	}
	
	return 0;
}
