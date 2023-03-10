///#############################################################
///Directories, Files and Timestamps
///Name: scan_directory.c
///#Written by: Chigozie Muonagolu -  Febuary 2022
/// 
///Purpose: To find files in a specified directory that are the largest, smallest, the most recent and the oldest modified file
/// 
///Usage : ./scan_directory.c "File_directory" or ./scan_directory.c
///
///Description of Parameters : argv[1] - The specified path to search
/// 
///Subroutines / libraries required : None
///
///#############################################################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

struct fileData {
	char rights[11];
	int numLinks;
	char owner[20];
	char group[20];
	int size;
	time_t date;
	char fileName[120];
}LargestData, smallestData, recentData, oldestData;
//Searches the directory recursively
int searchDirectory(DIR* dp, struct dirent* direct);
//Compares the data in filedate to the current file in the directory 
void compare(struct dirent* direct, struct stat stat3, int size, time_t  date);
//Populates the filedata with the appropiate data
void populate(struct fileData* file, char rights[], int numLinks, char owner[], char group[], int size, time_t date, char fileName[]);
//Prints the data in filedata in the appropriate format
void printData(struct fileData file, struct dirent* direct);
//Fuction to return a char containing the owner using the userid 
const char* getOwner(struct stat stat3);
//Fuction to return a char containing the group using the groupid 
const char* getGroup(struct stat stat3);
//Generate File permission
const char* getPermission(struct dirent* direct);

int main(int argc, char* argv[])
{
	char dir[120];
	DIR* dp;
	char currDir[120];

	//If no arugment was passed in the command line, search the current directory only then exit
	if (argv[1] == NULL)
	{
		//Opens up the current working directory instead
		if (getcwd(dir, 120) != NULL)
		{
			printf("Directory: %s\n", dir);
		}
	}
	//Else it searches the specified directory
	else
	{
		strncpy(dir, argv[1], 120);
		printf("Directory: %s\n", dir);
	}
	//Checks if the directory can be opened
	if ((dp = opendir(dir)) == NULL)
	{
		printf("Cannot open specified file\n");
		exit(1);
	}
	struct dirent* direct;		//To open the file 
	struct stat currStat;		//The current stats we are comparing
	if ((direct = readdir(dp)) == NULL)
	{
		printf("An error occured while trying to open read the directory.");
		exit(0);
	}
	searchDirectory(dp, direct);
	closedir(dp);
	printf("Largest file is: \n");
	printData(LargestData, direct);
	printf("Smallest file is: \n");
	printData(smallestData, direct);
	printf("Most recently modified file is: \n");
	printData(recentData, direct);
	printf("The oldest modified file is: \n");
	printData(oldestData, direct);
	return 0;
}

int searchDirectory(DIR* dp, struct dirent* direct)
{
	//Base case
	if (direct == NULL)
	{
		return 0;
	}
	struct stat stat2;
	if (stat(direct->d_name, &stat2) == 0)
	{
		//If the current data is a file
		if (S_ISREG(stat2.st_mode))
		{
			compare(direct, stat2, stat2.st_size, stat2.st_mtime);

		}
		else if (S_ISDIR(stat2.st_mode))
		{
			printf("Skipped directory: %s\n", direct->d_name);
		}
	}
	else
	{
		printf("Error occured while getting file/directory stats\n");
	}
	searchDirectory(dp, direct = readdir(dp));
	return 0;
}
void compare(struct dirent* direct, struct stat stat3, int size, time_t  date)
{
	char rights[11]="";
	int numLinks = stat3.st_nlink;
	char owner[20];
	char group[20];
	char fileName[120];
	strncpy(owner, getOwner(stat3), 20);
	strncpy(group, getGroup(stat3), 20);
	strncpy(fileName, direct->d_name, 120);

	if (LargestData.size == 0 && smallestData.size == 0 && recentData.size == 0 && oldestData.size == 0)
	{
		populate(&LargestData, rights, numLinks, owner, group, size, date, fileName);
		populate(&smallestData, rights, numLinks, owner, group, size, date, fileName);
		populate(&recentData, rights, numLinks, owner, group, size, date, fileName);
		populate(&oldestData, rights, numLinks, owner, group, size, date, fileName);
	}
	else
	{
		//Compares the largest, smallest, most recent and oldest modified files
		if (size >= LargestData.size)
		{
			populate(&LargestData, rights, numLinks, owner, group, size, date, fileName);
		}
		else if (size < smallestData.size)
		{
			populate(&smallestData, rights, numLinks, owner, group, size, date, fileName);
		}

		if (date >= recentData.date)
		{
			populate(&recentData, rights, numLinks, owner, group, size, date, fileName);
		}
		else if (date < oldestData.date)
		{
			populate(&oldestData, rights, numLinks, owner, group, size, date, fileName);
		}
	}
}
void populate(struct fileData* file, char rights[], int numLinks, char owner[], char group[], int size, time_t date, char fileName[])
{
	file->numLinks = numLinks;
	strncpy(file->owner, owner, 20);
	strncpy(file->group, group, 20);
	file->size = size;
	file->date = date;
	strncpy(file->fileName, fileName, 120);

}
const char* getTime(time_t date)
{
	struct tm* time;
	char* datebuf = (char*)malloc(sizeof(char) * 26);
	time = localtime(&date);
	strftime(datebuf, 26, "%Y-%m-%d %H:%M:%S", time);
	return datebuf;
}
const char* getName(struct fileData file)
{
	//Sets the file name to the full file name + its path
	char slash[30] = "/";
	strncat(slash, file.fileName, strlen(file.fileName));

	char* path = (char*)malloc(sizeof(char) * 120);
	getcwd(path, 120);

	strncat(path, slash, strlen(slash));
	return path;
}
const char* getOwner(struct stat stat3)
{
	char* fileOwner = (char*)malloc(sizeof(char) * 20);
	struct passwd* pw;
	pw = getpwuid(stat3.st_uid);
	strncpy(fileOwner, pw->pw_name, 20);
	return fileOwner;
}
const char* getGroup(struct stat stat3)
{
	char* fileGroup = (char*)malloc(sizeof(char) * 20);
	struct group* gd;
	gd = getgrgid(stat3.st_gid);
	strncpy(fileGroup, gd->gr_name, 20);
	return fileGroup;
}
const char* getPermission(struct dirent* direct)
{
	struct stat stats;
	stat(direct->d_name, &stats);

	int i = 0;
	char* filePermisson = (char*)malloc(sizeof(char) * 11);
	filePermisson[i] = '-';
	++i;
	//For USER
	if (stats.st_mode & S_IRUSR)
	{
		filePermisson[i] = 'r';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IWUSR)
	{
		filePermisson[i] = 'w';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IXUSR)
	{
		filePermisson[i] = 'x';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	//For GROUP
	if (stats.st_mode & S_IRGRP)
	{
		filePermisson[i] = 'r';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IWGRP)
	{
		filePermisson[i] = 'w';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IXGRP)
	{
		filePermisson[i] = 'x';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	//For OTHER
	if (stats.st_mode & S_IROTH)
	{
		filePermisson[i] = 'r';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IWOTH)
	{
		filePermisson[i] = 'w';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	if (stats.st_mode & S_IXOTH)
	{
		filePermisson[i] = 'x';
		++i;
	}
	else
	{
		filePermisson[i] = '-';
		++i;
	}
	return filePermisson;
}

void printData(struct fileData file, struct dirent* direct)
{
	strncpy(file.rights, getPermission(direct), 10);
	printf("%s %d %s %s	%d bytes	%s	%s\n", file.rights, file.numLinks, file.owner, file.group, file.size, getTime(file.date), getName(file));
}