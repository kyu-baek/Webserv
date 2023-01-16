# Mini-WebServer

For building a fully operating mini-webserver.
This project is a team-work.
<br>
webserver connection part = up to me
<br>
server config parser = up to kyulim
<br>
request parser = up to nttmxk
<br>
Config class defines server config; <br>
HttpServer class defines server creation and inlcudes the whloe logic of server; <br>

```
	if (webServer.openServer() == FAIL)
		return (1);

	if (webServer.runServer() == FAIL) //runServer() includes Connection class methods
		return (1);
```
webServer.openServer() : socket() + bind() + listen(); <br>
webServer.runServer() : accept() + read() + write(); <br>

<br>
<h2> GET </h2>
/home : index.html <br>
/server : server.html <br>
<h2> POST </h2>
/submit : cgi request to submit.py<br>
/upload : cgi request to upload.py<br>

