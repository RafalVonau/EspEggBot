/*
 * HTTPCommand - implements a web page and allows to execute eggbot commands from a web browser (direct printing of SVG files).
 *
 * NOTE: The class uses a script written in python and hosted at https://tymek.duckdns.org/eggbot/togcode.py.
 *       to convert the SVG file into GCODE commands.
 *
 * Author: Rafal Vonau <rafal.vonau@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 */
#include "HTTPCommand.h"
#include "www_fs.h"

//====================================================================================
//=============================-- NETWORK EVENTS--====================================
//====================================================================================

static void notFound(AsyncWebServerRequest *request)
{
	request->send(404, "text/plain", "Not found");
}
//====================================================================================

static void handle_request(AsyncWebServerRequest *request, char *type, const uint8_t * data, int len)
{
	AsyncWebServerResponse *response = request->beginResponse_P(200, type, data, len);
	response->addHeader("Content-Encoding", "gzip");
	request->send(response);
}
//====================================================================================

HTTPCommand::HTTPCommand(CommandDB *db): Command(db)
{
	m_server = new AsyncWebServer(80);
	m_events = new AsyncEventSource("/events");

	m_server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/html", __index_html, www_index_html_size);});
	/* Java Script */
	m_server->on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/javascript", __main_js, www_main_js_size);});
	m_server->on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/javascript", __jquery_min_js, www_jquery_min_js_size);});
#if defined(www_bezier_js_size)
	m_server->on("/bezier.js", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/javascript", __bezier_js, www_bezier_js_size);});
#endif
#if defined(www_nanosvg_js_size)
	m_server->on("/nanosvg.js", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/javascript", __nanosvg_js, www_nanosvg_js_size);});
#endif
	/* CSS */
	m_server->on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/css", __main_css, www_main_css_size);});
#if defined(www_bulma_min_css_size)
	m_server->on("/bulma.min.css ", HTTP_GET, [](AsyncWebServerRequest *request){handle_request(request, "text/css", __bulma_min_css, www_bulma_min_css_size);});
#endif
	m_server->on("/post", HTTP_POST, [this](AsyncWebServerRequest *request) {
		String message;
		if (request->hasParam("cmd", true)) {
			message = request->getParam("cmd", true)->value() + "\r";
			handleData(message.c_str(), message.length());
		} else {
			message = "No message sent";
		}
		request->send(200, "text/plain", message);
	});
	m_server->onNotFound(notFound);

	m_events->onConnect([](AsyncEventSourceClient *client) {client->send("hello!",NULL,millis(),1000);});
	m_server->addHandler(m_events);
	m_server->begin();
}
//====================================================================================

HTTPCommand::~HTTPCommand() 
{
	delete m_events;
	delete m_server;
}
//====================================================================================

void HTTPCommand::handleData(const char *data, int n)
{
	int i;
	
	for (i=0; i < n; ++i) {
		char inChar = data[i];
		if ((inChar == '\r') || (inChar == '\n')) {
			if (bufPos) {
				buffer[bufPos] = '\0';
				m_db->executeCommand(this, buffer);
				clearBuffer();
			}
		} else if (isprint(inChar)) {     // Only printable characters into the buffer
			if (bufPos < COMMAND_BUFFER) {
				buffer[bufPos++] = inChar;  // Put character into buffer
			}
		}
	}
}
//====================================================================================
