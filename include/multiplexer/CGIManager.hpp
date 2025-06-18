#pragma once
# define BUFFER_SIZE 30720
# include <iostream>
# include <map>
# include <unistd.h>
# include <cstdlib>

struct	CGIInfo
{
	int clientFd;
	int pipeFd;
	pid_t pid;
	std::string output;
	bool isFinished;
};

class	CGIManager
{
	public:
		CGIManager();
		~CGIManager();
		std::map<int, CGIInfo*>	_cgiProcesses;

		void	addCgiProcess(int clientFd, int pipeFd, pid_t pid);
		bool	isInManager(int pipeFd);
		void	readCGIOutput(int pipeFd);
		void	deleteFromCGI(int pipeFd);
		bool	isResponseComplete(int pipefd);
};