#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

typedef struct section{
	char sect_name[18];
	int sect_type;
	int sect_offset;
	int sect_size;
}Section;

typedef struct sf {
	char magic[4];
	short header_size;
	short version;
	char nb_of_sections;
	Section *sections[1000];
}SF;

void freeSF(SF *sf)
{
	for (int i = 0; i < sf->nb_of_sections; i++)
		free(sf->sections[i]);
	free(sf);
}

void parseResult(int parse,SF *sf) {
	if (parse == 0)
	{
		printf("SUCCESS\nversion=%d\nnr_sections=%d\n", sf->version, sf->nb_of_sections);
		for (int i = 0; i < sf->nb_of_sections; i++)
			printf("section%d: %s %d %d\n", i + 1, sf->sections[i]->sect_name, sf->sections[i]->sect_type, sf->sections[i]->sect_size);	
	}
	else
		if(parse==-1)
			printf("ERROR\nwrong magic\n");
		else
			if(parse==-2)
				printf("ERROR\nwrong version\n");
			else
				if(parse==-3)
					printf("ERROR\nwrong number of sections\n");
				else
					if(parse==-4)
						printf("ERROR\nwrong sect_types\n");
}

//parse
int parse(char *path, SF *sf)
{
	int file_descriptor = open(path, O_RDONLY);

	read(file_descriptor, sf->magic, 4);
	if (strcmp(sf->magic, "72dL") != 0)
		return -1;

	read(file_descriptor, &sf->header_size, 2);

	read(file_descriptor, &sf->version, 2);
	if (sf->version < 46 || sf->version> 162)
		return -2;

	read(file_descriptor, sf->nb_of_sections, 1);
	if (sf->nb_of_sections < 7 || sf->nb_of_sections>20)
		return -3;

	Section *sections[1000] = (Section*)malloc(1000 * sizeof(Section));
	for (int i = 0; i < sf->nb_of_sections)
	{
		Section *sect_header = (Section*)malloc(sizeof(Section));
		read(file_descriptor, sect_header->sect_name, 18);
		read(file_descriptor, sect_header->sect_type, 4);
		if (sect_header->sect_type != 62 &&
			sect_header->sect_type != 91 &&
			sect_header->sect_type != 42 &&
			sect_header->sect_type != 54 &&
			sect_header->sect_type != 33)
			return -4;
		read(file_descriptor, sect_header->sect_offset, 4);
		read(file_descriptor, sect_header->sect_size, 4);
		sections[i] = sect_header;
	}
	sf->sections = sections;
	
	return 0;
}

int checkSection(char *path,SF *sf) {
	if (parse(path, sf) == 0)
	{
		int k = 0;
		for (int i = 0; i < sf->nb_of_sections; i++)
			if (sf->sections[i]->sect_type == 33)
				k++;
		if (k >= 2)
			return 0;
	}
	return -1;
}

int fct(char *perm)
{
	int result = 0;
	if (perm[0] == 'r')
		result += 4;
	if (perm[1] == 'w')
		result += 2;
	if (perm[2] == 'x')
		result += 1;
	return result;
}

int checkThisPerm(char *path, char *perm)
{
	struct stat forFile;
	
	lstat(path, &forFile);
	int fileMask = forFile.st_mode;
	int mask = 0000;
	char p1[3], p2[3];
	strncpy(p1, perm, 3);
	strncpy(p2,perm + 3, 3);
	mask+=fct(p1) * 64 + fct(p2) * 8 + fct(perm + 6);
	if (fileMask == mask)
		return 0;
	return -1;
}

void print(DIR *dir, char *path,int recursive,char *name,int sf,char *permissions)
{
	struct dirent *inodes;
	while (inodes == readdir(dir))
	{
		if(strcmp(inodes->d_name,".")!=0)
			if (strcmp(inodes->d_name, "..") != 0)
			{
				char *path2;
				snprintf(path2, 100, "%s/%s", path, inodes->d_name);
				if (strcmp(name, "") != 0 )
				{
					if (strcmp(path2 + strlen(path2) - strlen(name),name) == 0)
						printf("%s\n", path2);
				}
				if (sf == 1)
				{
					SF *newSF=(SF*)malloc(sizeof(SF));
					if(checkSection(path2,newSF)==0)
						printf("%s\n",path2);
					freeSF(newSF);
				}
				if (strcmp(permissions, "") != 0)
				{
					if (checkThisPerm(path2, permissions) == 0)
						printf("%s\n", path2);
				}
				if (recursive == 1)
				{
					struct stat info;
					lstat(path2, &info);
					if (S_ISDIR(info.st_mode))
					{
						DIR *newdir = opendir(path2);
						print(newdir, path2, recursive,permissions);
						closedir(newdir);
					}
				}
			}
	}
	free(inodes);
}

//recursive
int recursive(char *path,int recursive,char *name,int sf,char *permissions)
{
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		printf("ERROR\ninvalid directory path\n");
		closedir(dir);
		return -1;
	}
	printf("SUCCESS\n");

	print(dir, path,recursive,name,sf,permissions);
	closedir(dir);
	return 0;
}

//extract
int extract(char *path,int section, int line)
{
	SF *sf = (SF*)malloc(sizeof(SF));

	int file_descriptor = open(path, O_RDONLY);

	if (file_descriptor == 0 || parse(path,sf)!=0)
	{
		printf("ERROR\ninvalid file\n");
		return-1;
	}

	if (sf->nb_of_sections < section)
	{
		printf("ERROR\ninvalid section\n");
		return -1;
	}
	if (line < 1)
	{
		printf("ERROR\ninvalid line\n");
		return -1;
	}

	lseek(file_descriptor, sf->sections[section-1]->sect_offset,SEEK_SET);

	char *buffer=(char*)malloc(sf->sections[section-1]->sect_size*sizeof(char));

	read(file_descriptor, buffer, sf->sections[section - 1]->sect_size);

	int ln = 1;
	char *p;
	p = strtok(buffer,0x0A);
	while (ln < line)
	{
		ln++;
		p = strtok(NULL, 0x0A);
	}
	if (ln < line)
	{
		free(p);
		free(buffer);
		printf("ERROR\ninvalid line\n");
		return -1;
	}
	
	printf("SUCCESS\n%s\n", p);
	free(p);
	free(buffer);

	freeSF(sf);
	return 0;
}

//find all
int fct4(char *path) {

	recursive(path, 1, "", 1);
}

int search(int argc, char **argv, char *name)
{
	for (int i = 1; i < argc; i++)
		if (strstr(argv[i], name) == 1)
			return i;
	return 0;
}

int checkPerm(char* perm)
{
	if (strlen(perm) != 9)
		return -1;
	if (perm[0] != 'r' && perm[0] != '-' && perm[3] != 'r' && perm[3] != '-' && perm[6] != 'r' && perm[6] != '-')
		return -1;
	if (perm[1] != 'w' && perm[1] != '-' && perm[4] != 'w' && perm[4] != '-' && perm[7] != 'w' && perm[7] != '-')
		return -1;
	if (perm[2] != 'x' && perm[2] != '-' && perm[5] != 'x' && perm[5] != '-' && perm[8] != 'x' && perm[8] != '-')
		return -1;
	return 0;
}

int main(int argc, char **argv){
	if (argc >= 2) {
		if (strcmp(argv[1], "variant") == 0) {
			printf("33085\n");
		}
		else
			if ( (int poz=search(argc,argv,"list") )!= 0)
			{
				int perm = search(argc, argv, "permissions=");
				char* permission;
				if (checkPerm(argv[perm] + strlen("permissions=") != 0)
					strcpy(permission,"");
				else
					strcpy(permission, argv[perm] + strlen("permissions="));
				
				recursive(argv[poz], search(argc,argv,"recursive") != 0 ? 1 : 0, 
				(int p1=search(argc,argv,"name_ends_with=")) != 0 ? argv[p1]+strlen("name_ends_with=") : "",
				permission);
			}
			else
				if ((int p2=search(argc,argv,"parse")) != 0)
				{
					SF *sf = (SF*)malloc(sizeof(SF));
					int pth = search(argc, argv, "path=");
					if (pth == 0)
					{
						printf("ERROR\ninvalid path\n");
						return -1;
					}
					int parse=parse(argv[pth]+strlen("path="),sf);
					parseResult(parse, sf);
					freeSF(sf);
				}
				else
					if (search(argc,argv,"extract") != 0)
					{
						int sect = search(argc, argv, "section=");
						if(sect==0)
						{
							printf("ERROR\ninvalid section\n");
							return -1;
						}
						int lin = search(argc, argv, "line=");
						if (lin == 0)
						{
							printf("ERROR\ninvalid line\n");
							return -1;
						}
						int pth = search(argc, argv, "path=");
						if (pth == 0)
						{
							printf("ERROR\ninvalid path\n");
							return -1;
						}
							extract(argv[pth],atoi(argv[sect]+strlen("section=")),atoi(argv[lin]+strlen("line=")));
					}
					else
						if (search(argc,argv,"findall") != 0)
						{
							int pth = search(argc, argv, "path=");
							if (pth == 0)
							{
								printf("ERROR\ninvalid path\n");
								return -1;
							}
							fct4(argv[pth]+strlen("path="));
						}
						else
						{
							printf("ERROR\ninvalid line\n");
							return -1;
						}
	}
    return 0;
}