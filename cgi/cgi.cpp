/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: del-yaag <del-yaag@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/18 02:15:25 by mmisskin          #+#    #+#             */
/*   Updated: 2024/03/31 12:58:59 by del-yaag         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include "../client/Tools.hpp"
#include <unistd.h>
#include <signal.h>

# define CGIIN "/tmp/.cgiInput"
# define CGIOUT "/tmp/.cgiOutput"

/* ********************************************* utility functions *************************************** */
std::string	getIdentifier(std::string header)
{
	/* remove the : at the end (Header':') */
	header.pop_back();

	for (size_t i = 0; header[i]; i++)
	{
		if (header[i] >= 'a' && header[i] <= 'z')
			header[i] = std::toupper(header[i]);
		else if (header[i] == '-')
			header[i] = '_';
	}
	header = "HTTP_" + header;

	return (header);
}

std::vector<std::string>	buildArgv(std::string const & cgi, std::string const & script)
{
	std::vector<std::string>	argv;

	if (cgi.empty())
		argv.push_back("./" + script);
	else
	{
		argv.push_back(cgi);
		argv.push_back(script);
	}
	return (argv);
}

std::vector<std::string>	buildEnv(std::map<std::string, std::string> & req, std::string const & method, std::string const & protocol, std::string const & url)
{
	/* setup meta-variables */
	std::map<std::string, std::string>::iterator	it;
	std::vector<std::string>						environment;
	std::string										queryString;

	environment.push_back("REQUEST_METHOD=" + method);
	environment.push_back("SERVER_PROTOCOL=" + protocol);
	if (url.find('?') != std::string::npos)
	{
		environment.push_back("QUERY_STRING=" + url.substr(url.find('?') + 1));
		environment.push_back("PATH_INFO=" + url.substr(0, url.find('?')));
	}
	else
	{
		environment.push_back("QUERY_STRING");
		environment.push_back("PATH_INFO=" + url);
	}
	if ((it = req.find("Content-Type:")) != req.end())
		environment.push_back("CONTENT_TYPE=" + it->second);

	std::string	entry;
	for (it = req.begin(); it != req.end(); it++)
	{
		if (it->first == "Content-Type:" || it->first == "Content-Length:")
			continue ;
		if (it->second.empty())
			entry = getIdentifier(it->first);
		else
			entry = getIdentifier(it->first) + "=" + it->second;
		environment.push_back(entry);
	}
	return (environment);
}

/* converts a vector to a (char **) */
char	**Convert(std::vector<std::string> const & vec)
{
	size_t	size = vec.size() + 1;
	char **newVec = new char *[size];
	newVec[size - 1] = NULL;
	for (size_t i = 0; i < vec.size(); i++)
	{
		newVec[i] = new char [vec[i].length() + 1];
		newVec[i][vec[i].length()] = 0;
		vec[i].copy(newVec[i], vec[i].length());
	}
	return (newVec);
}

std::string	getScriptName(std::string path)
{
	size_t	pos;
	size_t	len;
	size_t	start;

	/* find the first extension */
	pos = path.find('.');

	/* go back till u hit a / */
	start = path.rfind('/', pos) + 1;

	/* calculate the length ]x, x[ or ]x, end] (x = {/, ?}) */
	len = path.find_first_of("/?", start) - start;

	return (path.substr(start, len));
}
/* ***************************************************************************************************** */

/* cgi methods */
Cgi::Cgi(void) : _isSet(false), _started(false), _post(false), _ready(true) {}
bool				Cgi::isSet(void) const { return (_isSet); }
void				Cgi::enable(void) { _isSet = true; }
bool				Cgi::isStarted(void) const { return (_started); }
bool				Cgi::ready(void) const { return (_ready); }
void				Cgi::setPid(pid_t const pid) { _pid = pid; }
void				Cgi::setCgiTime(size_t time) { _cgiTime = time; }
void				Cgi::setCgiStdErr(int stdErr) { _cgiStdErr = stdErr; }
void				Cgi::setScriptName(std::string const & name) { _scriptName = name; }
void				Cgi::setCwd(std::string const & cwd) { _cwd = cwd; }
void				Cgi::setEnv(std::vector<std::string> const & env) { _env = env; }
void				Cgi::setArgv(std::vector<std::string> const & argv) { _argv = argv; }
void				Cgi::setPost(bool post) { _post = post; }
void				Cgi::setReady(bool ready) { _ready = ready; }
pid_t				Cgi::getPid(void) const { return (_pid); }
size_t				Cgi::getCgiTime(void) const { return (_cgiTime); }
int					Cgi::getCgiStdErr(void) const { return (_cgiStdErr); }
std::string const &	Cgi::getCgiInFile(void) const { return (_cgiInFile); }
std::string const &	Cgi::getCgiOutFile(void) const { return (_cgiOutFile); }
void				Cgi::setFiles(std::string const & suffix)
{
	_cgiInFile = CGIIN + suffix;
	_cgiOutFile = CGIOUT + suffix;
}

void				Cgi::launch(std::string const & sessionId, std::string const & cookie)
{
	if (_post && !_ready)
		return ;
	else if (_post)
	{
		/* calculate content-length and add it to the meta variables */
		std::ifstream	cgiIn(_cgiInFile);

		if (!cgiIn.good())
			return ;
		cgiIn.seekg(0, cgiIn.end);
		std::streampos	length = cgiIn.tellg();
		_env.push_back("CONTENT_LENGTH=" + intToString(static_cast<size_t>(length)));
		cgiIn.close();
	}

	/* set session id meta variable */
	if (!sessionId.empty())
		_env.push_back("X_ID=" + sessionId);
	else
		_env.push_back("X_ID=" + cookie);

	int	end[2];
	if (pipe(end) == -1)
	{
		std::cerr << "Error: cgi pipe: " << strerror(errno) << std::endl;
		return ;
	}

	if (fcntl(end[0], F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << strerror(errno) << std::endl;
		close(end[0]);
		close(end[1]);
		return ;
	}
	if (fcntl(end[1], F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << strerror(errno) << std::endl;
		close(end[0]);
		close(end[1]);
		return ;
	}

	pid_t	pid = fork();
	if (pid == -1)
	{
		std::cerr << "Error: cgi: " << strerror(errno) << std::endl;
		close(end[1]);
		close(end[0]);
		return ;
	}
	if (!pid)
	{
		/* std::cout << this->path << std::endl; */
		/* std::cout << this->url << std::endl; */
		/* std::cout << cgi << std::endl; */
		/* std::cout << path.substr(path.find('.')) << std::endl; */
		/* std::string	cwd = itLocation->second.getRoot().getPath() + itLocation->first + path.substr(1, path.rfind('/')); */
		/* std::cout << cwd << std::endl; */
		/* std::cout << script << std::endl; */

		/* change the child's working directory */
		chdir(_cwd.c_str());

		/* set the script's environment */
		char **env = Convert(_env);

		char **av = Convert(_argv);

		/* std::cerr << "before" << std::endl; */
		close(end[0]);
		dup2(end[1], STDERR_FILENO);
		close(end[1]);
		/* std::cerr << "after" << std::endl; */

		if (_post)
		{
			int in = open(_cgiInFile.c_str(), O_RDONLY);
			if (in == -1)
			{
				std::cerr << "cgi: input file opening failed" << std::endl;
				exit(1);
			}
			dup2(in, STDIN_FILENO);
			close(in);
		}

		/* setup and change child's standart output */
		int fd = open(_cgiOutFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd == -1)
		{
			std::cerr << "cgi: output file opening failed" << std::endl;
			exit(1);
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);

		execve(av[0], av, env);
		std::cerr << "execve: " << strerror(errno) << std::endl;
		exit(1);
	}

	/* close the write end since we're not gonna use it */
	close(end[1]);

	_pid = pid;
	_cgiTime = std::time(NULL);
	_cgiStdErr = end[0];
	_started = true;
}

bool	Request::handleCgiRequest(std::string const & root, std::string const & location, std::string const & cgi, Response & response)
{
	if (this->method == "DELETE")
	{
		response.setStatusCode(501);
		return (false);
	}
	else if (this->method == "POST")
	{
		this->cgi.setPost(true);
		this->cgi.setReady(false);
	}

	std::string	script = getScriptName(path);
	std::string	cwd;
	if (!isDirectory(location.c_str()))
		cwd = root + '/';
	else
		cwd = root + location;

	if (access((cwd + script).c_str(), F_OK) != 0)
	{
		response.setStatusCode(404);
		return (false);
	}

	this->cgi.setScriptName(script);
	this->cgi.setCwd(cwd);
	this->cgi.setEnv(buildEnv(this->linesRequest, this->method, this->httpVersion, this->url));
	this->cgi.setArgv(buildArgv(cgi, script));

	return (true);
}

/* 
 * check if the script has finished
 * if it finished send the response header
 * otherwise if the timeout or an error occurs kill it
 * if none of the above return 1
 */
int	monitorCgiProcess(Request & request, Response & response, int const sockfd)
{
	Cgi		cgi = request.getCgi();
	char	tmp[1025] = {0};
	int		err = read(cgi.getCgiStdErr(), tmp, 1024);

	if (!cgi.isStarted())
	{
		response.setStatusCode(500);
		return (1);
	}

	if (cgi.getPid() == waitpid(cgi.getPid(), NULL, WNOHANG))
	{
		close(cgi.getCgiStdErr());
		remove(cgi.getCgiInFile().c_str());
		if (response.sendCgiHeader(sockfd, request) == -1)
			return (0);
		else
			response.setSendedHeader( true );
	}
	else if (std::time(NULL) - cgi.getCgiTime() >= 15 || err > 0)
	{
		close(cgi.getCgiStdErr());
		kill(cgi.getPid(), SIGTERM);
		remove(cgi.getCgiOutFile().c_str());
		remove(cgi.getCgiInFile().c_str());

		if (err > 0)
			response.setStatusCode(500);
		else
			response.setStatusCode(504);
	}
	return (1);

}
