#include <windows.h>
#include <string>
#include <stdio.h>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS

const int PORT_NUM = 5555;	//port on which server listens, e.g. 'localhost:5555' will be used to connect

std::string getNumericalArgumentFromHTTPMessage(const std::string& HTTPMessage, const std::string& arg);
std::string craftHTTPHeader(const std::string& type);
void enableCOM(const std::string comnum);

// Run a Windows /SUBSYSTEM application (no command prompt or window)
int WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       cmdShow)
{
	/*
		- Overview: Main creates a local server at specified port PORT_NUM.
		- Working: When a client requests a connection to this server, it will SET a COM Port's RTS pin for 100ms.
		- Operation: This can be done by simply visiting 'localhost:PORT_NUM?COM=222' in a web browser. 
			Here, PORT_NUM is the global defined in this source file at compile-time. '222' is the COM port assigned to our USB converter at runtime. 
		- Note: The COM port varies, so the number must be confirmed and then supplied as the argument. E.g. 'localhost:5555?COM=8'
	*/
    WSADATA WSAData;
 
    SOCKET server, client;
 
    SOCKADDR_IN serverAddr, clientAddr;
 
    WSAStartup(MAKEWORD(2,0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);
 
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_NUM);	//port on which the server listens
 
    bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    listen(server, 0);
 
    char buffer[1024];
    int clientAddrSize = sizeof(clientAddr);

    while((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)	//This blocks until a client requests connection.
    {
        recv(client, buffer, sizeof(buffer), 0);	//recieve incoming message
		std::string COMPORT_str = getNumericalArgumentFromHTTPMessage(buffer, "COM");
		std::string response_str;	//the response to send back to the client

		if (!COMPORT_str.empty())	//argument was found
		{
			enableCOM(COMPORT_str); //signal the COM port
			//an HTTP '204 No Content' response is created so the client can be told their message was received, so they won't re-send the message multiple times.
			response_str = craftHTTPHeader("200 OK");
			
		}
		else    //argument wasn't found
		{
			//A '400 Bad Request' response is sent back.
			response_str = craftHTTPHeader("400 Bad Request");
		}

		send(client, response_str.c_str(), response_str.size(), 0);	//sends back the response

        memset(buffer, 0, sizeof(buffer));	//clear buffer that receives message
 
        closesocket(client);	//disconnects the client, as its only job is to signal to the server.
    }
}

std::string craftHTTPHeader(const std::string& type)
{
	std::string response_str;
	response_str = "HTTP/1.1 " + type + "\r\n";
	response_str += "Content-Type: text/plain\r\n";
	response_str += "Content-Length: " + std::to_string(type.length()) + "\r\n\r\n";
	response_str += type;
	return response_str;
}


std::string getNumericalArgumentFromHTTPMessage(const std::string& HTTPMessage, const std::string& arg)
{
	//returns Numerical part after any arg.
	// e.g. If 'localhost:5555?COM=444' is url visited, function argument 'arg' should be 'COM', then returned string would be '444'
	auto pos = HTTPMessage.find_first_of(arg);
	pos += arg.length() + 1; //an extra char for '=' as in 'COM='
	int len = 0;	//length of numerical part
	while (isdigit(HTTPMessage[pos]))	
	{
		len++;
		pos++;
		if (pos >= HTTPMessage.length())
		{
			len = 0;
			break;
		}
	}
	return HTTPMessage.substr(pos - len, len);	//return the numerical part against the given arg
}

void enableCOM(const std::string comnum)
{
	/*
		- Overview: Signals the RTS pin for 100ms.
		- Working: When this function is run, the RTS control pin at 'comnum' COM port will be set for 100ms before being unset.
		- Operation: Check runtime COM port assigned to your controller/converter.
		- Note: In many converters, RTS and DTR pins are inverted, meaning a SET RTS will be Voltage LOW, and a CLEARED RTS will be Voltage High.
			In context of LED, CLEAR RTS (default) will make the LED light up and SET RTS will make the light turn off.
	*/

	std::string ComPortN("\\\\.\\COM" + comnum);

	//boilerplate code below
	HANDLE hComm;                          // Handle to the Serial port
	const char* ComPortName = ComPortN.c_str();  // Name of the Serial port(May Change) to be opened,
	BOOL  Status;

	hComm = CreateFile(ComPortName,                  // Name of the Port to be Opened
		GENERIC_READ | GENERIC_WRITE, // Read/Write Access
		0,                            // No Sharing, ports cant be shared
		NULL,                         // No Security
		OPEN_EXISTING,                // Open existing port only
		0,                            // Non Overlapped I/O
		NULL);                        // Null for Comm Devices


	Status = EscapeCommFunction(hComm, SETRTS);

	//sleep 100ms
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point sleep = std::chrono::system_clock::now();
	while (std::chrono::duration_cast<std::chrono::milliseconds>(sleep - now).count() < 100)	//keeps the RTS pin ON for 100 ms or 0.1s Note: receiver may burn if kept ON for longer.
	{
		sleep = std::chrono::system_clock::now();
	}

	Status = EscapeCommFunction(hComm, CLRRTS);

	CloseHandle(hComm);//Closing the Serial Port

}