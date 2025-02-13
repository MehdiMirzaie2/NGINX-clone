#include "cgi.hpp"

static int calcContentLength(std::string const &message)
{
	std::stringstream ss(message);
	std::string line;

	std::getline(ss, line, '\n');
	return (message == "" ? message.size() - 1: message.size() - line.size() - 2);
}

Cgi::Cgi() {}

Cgi::~Cgi()
{
	std::vector<char *>::iterator it;
	for (it = _argv.begin(); it != _argv.end(); ++it)
	{
		free(*it);
	}
	for (it = _env.begin(); it != _env.end(); ++it)
	{
		free(*it);
	}
}

void Cgi::setEnv(HTTPRequest &req)
{
	_envVec.push_back("Content-Length=" + req.getHeader("Content-Length"));
	_envVec.push_back("User-Agent=" + req.getHeader("User-Agent"));
	_envVec.push_back("Content-Type=" + req.getHeader("Content-Type"));
	_envVec.push_back("User-Agent=" + req.getHeader("User-Agent"));
	_envVec.push_back("Method=" + req.getMethodString());
	// std::cout << BOLDBLUE << "method = " << req.getMethodString() << RESET << std::endl;

	std::string content_type = req.getHeader("Content-Type");

	if (!req.getQString().empty())
	{
		_envVec.push_back("QUERY_STRING=" + req.getQString());
	}

	// TODO: This should be a check for the method
	// It should only encode the query string if it is a get request (and not a post request)
	// This has been left as some legacy code expects a short post body to be encoded as a query string
	// if (content_type.size() >= 9 && content_type.substr(0, 9) == "multipart") {
	// 	// Do nothing
	// } else {
	// 	_envVec.push_back("QUERY_STRING=" + req.getBody());
	// }


	// _envVec.push_back("REQUEST_METHOD=" + req.getMethodString());
	_envVec.push_back("PATH_INFO=" + req.getUri());
	_envVec.push_back("SERVER_PROTOCOL=HTTP/1.1");

	if (!req.getHeader("Cookie").empty())
	{
		std::cout << req.getHeader("Cookie") << std::endl;
		_envVec.push_back("Cookie=" + req.getHeader("Cookie"));
	}
	// _env = (char **)malloc(sizeof(char *) * (_envVec.size() + 1));
	_env = std::vector<char *>(_envVec.size() + 1);
	std::vector<std::string>::iterator it;
	int i = 0;
	for (it = _envVec.begin(); it != _envVec.end(); it++, i++)
	{
		_env[i] = new char[(*it).size() + 1];
		strcpy(_env[i], const_cast<char *>(it->c_str()));
	}
	_env[i] = nullptr;
}

void Cgi::setArgv(HTTPRequest const &req)
{
	_argv = std::vector<char *>(3);
	std::string python_path = "/usr/local/bin/python3";
	std::string perl_path = "/usr/bin/perl";
	std::string exec_path = "";

	std::string uri = req.getUri();

	if (uri == "/cgi-bin/cgi_tester")
	{
		exec_path = "application/cgi-bin/cgi_tester";
	}
	else if (uri.size() >= 3 && uri.substr(uri.size() - 3, uri.size()) == ".py")
	{
		exec_path = python_path;
	}
	else if (uri.size() >= 3 && uri.substr(uri.size() - 3, uri.size()) == ".pl")
	{
		exec_path = perl_path;
	}
	else
	{
		//TODO probably should throw or return 403 or something here
	}

	std::string cgiScript = "application" + req.getUri();

	_argv[0] = new char[exec_path.size() + 1];
	_argv[1] = new char[cgiScript.size() + 1];

	strcpy(_argv[0], const_cast<char *>(exec_path.c_str()));
	strcpy(_argv[1], const_cast<char *>(cgiScript.c_str()));
	_argv[2] = nullptr;
}


void Cgi::CgiReadHandler(ServerManager &sm, Client *cl, struct kevent ev_list)
{
	char buffer[ev_list.data + 1];
	int bytesRead = 0;
	int status;

	memset(buffer, 0, ev_list.data + 1);
	bytesRead = read(ev_list.ident, buffer, ev_list.data);
	RECORD("cgiReadHandler: Read: %s", buffer);
	if (bytesRead < 0)
	{
		ERR("cgiReadHandler: error");
		sm.updateEvent(ev_list.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		close(cl->_pipe_in[0]);
		close(cl->_pipe_out[0]);
		DEBUG("PIPES ends closing: %d %d", cl->pipe_in[0], cl->pipe_out[0]);
	}
	else if (bytesRead == 0)
	{
		DEBUG("cgiReadHandler: Bytes Read = 0");
		sm.updateEvent(ev_list.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		close(cl->_pipe_in[0]);
		close(cl->_pipe_out[0]);

		waitpid(cl->_Cgipid, &status, WCONTINUED);
		if (WIFEXITED(status))
		{
			DEBUG("cgi exit status = %d", WEXITSTATUS(status));
			if (WEXITSTATUS(status) == 1)
			{
				// error case
				handleTerminatedWithError(sm, cl);
				return ;
			}
		}
		if (!WIFCONTINUED(status))
			handleSuccessfulTermination(sm, cl);
	}
	else
	{
		DEBUG("Bytes Read: %d", bytesRead);
		cl->appendRecvMessage(buffer, bytesRead);
		if (waitpid(cl->_Cgipid, &status, WNOHANG) == 0)
		{
			if (cl->getRecvMessage().size() >= MAX_CONTENT_LENGTH)
			{
				kill(cl->_Cgipid, SIGQUIT);
				handleTerminatedWithError(sm, cl);
			}
		}
		else
		{
			waitpid(cl->_Cgipid, &status, WCONTINUED);
			RECORD("cgi exit status = %d", WEXITSTATUS(status));
			if (WIFEXITED(status))
			{
				if (WEXITSTATUS(status) == 1)
					handleTerminatedWithError(sm, cl);
			}
			else
				return ;
			if (!WIFCONTINUED(status))
				handleSuccessfulTermination(sm, cl);
		}
	}
}

void Cgi::handleSuccessfulTermination(ServerManager &sm, Client *cl)
{
	HTTPResponse cgiResponse;

	cgiResponse.setBody(cl->getRecvMessage());
	cgiResponse.setVersion("HTTP/1.1");
	cgiResponse.addHeader("Content-Type", "text/HTML");
	int contentlen = calcContentLength(cl->getRecvMessage());
	cgiResponse.addHeader("Content-Length", std::to_string(contentlen));
	cgiResponse.setCgiStatus(true);
	cl->setMessage(Message(cgiResponse));

	sm.updateEvent(cl->getSockFD(), EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	sm.updateEvent(cl->getSockFD(), EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
}

void Cgi::handleTerminatedWithError(ServerManager &sm, Client *cl)
{
	HTTPResponse cgiResponse;

	cgiResponse.setCgiStatus(false);
	cgiResponse.getErrorResource(500);
	cl->setMessage(Message(cgiResponse));
	sm.updateEvent(cl->getSockFD(), EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	sm.updateEvent(cl->getSockFD(), EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
}

bool Cgi::CgiWriteHandler(ServerManager &sm, Client *cl, struct kevent ev_list)
{
	// RECORD("WRITING");
	int bytes_sent;
	if (cl == NULL)
		return false;

	Message message = cl->getMessage();
	if (message.size() == 0)
	{
		bytes_sent = 0;
		DEBUG("Body sent to CGI-Script: %s", message.getMessage().c_str() + message.getBufferSent());
	}
	else if (message.size() >= BUFFERSIZE)
	{
		bytes_sent = write(ev_list.ident, message.getMessage().c_str() + message.getBufferSent(), BUFFERSIZE);
		DEBUG("Body sent to CGI-Script: %s", message.getMessage().c_str() + message.getBufferSent());
	}
	else
	{
		bytes_sent = write(ev_list.ident, message.getMessage().c_str() + message.getBufferSent(), message.size());
		DEBUG("Body sent to CGI-Script: %s", message.getMessage().c_str() + message.getBufferSent());
	}

	if (bytes_sent < 0)
	{
		ERR("Unable to send Body to CGI-Script");
		return false;
	}

	else if (bytes_sent == 0 || bytes_sent == message.size())
	{
		cl->setMessage(Message());
		sm.updateEvent(cl->_pipe_out[0], EVFILT_READ, EV_ENABLE, 0, 0, NULL);
		return false;
	}
	else
	{
		message.setMessage(message.getMessage().substr(bytes_sent));
		cl->setMessage(message);
	}
	return true;
}

void Cgi::launchCgi(HTTPRequest &req, Client *cl)
{
	setArgv(req);
	setEnv(req);

	DEBUG("stepping into launchCgi");
	if (pipe(cl->_pipe_out) < 0)
	{
		ERR("Failed pipe_out cgi");
		return;
	}
	if (pipe(cl->_pipe_in) < 0)
	{
		ERR("Failed pipe_in cgi");
		return;
	}
	// Fork to create a child process for the CGI script
	cl->_Cgipid = fork();
	if (cl->_Cgipid == 0)
	{
		dup2(cl->_pipe_in[0], STDIN_FILENO);
		dup2(cl->_pipe_out[1], STDOUT_FILENO);
		close(cl->_pipe_out[0]);
		close(cl->_pipe_out[1]);
		close(cl->_pipe_in[0]);
		close(cl->_pipe_in[1]);
		execve(_argv[0], _argv.data(), _env.data());
		// If execl fails
		ERR("execve: %s %s", _argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if (cl->_Cgipid > 0)
	{
		close(cl->_pipe_in[0]);
		close(cl->_pipe_out[1]);
	}
	else
	{
		ERR("Failed to fork: %s", strerror(errno));
	}
}
