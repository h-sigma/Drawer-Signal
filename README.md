# Drawer-Signal
A program to operate an electronic drawer through an USB-to-TTL Converter. Currently supports Windows using the Win10 SDK and the winsock2 API. The /SUBSYSTEM option must be enabled for it to compile.

# COM Ports
The USB-to-TTL Converter will create a Virtual COM Port in your Windows OS, which enables the traditional Serial COM Ports to connect to your PC through a USB port. Each Virtual COM port is assigned a number that is not known until run-time/connection-time. So, when you use the utility, you must first find out the COM Port number assigned to your device. 

How to find out your COM Port number and Further Reading: 
https://tnp.uservoice.com/knowledgebase/articles/172101-determining-the-com-port-of-a-usb-to-serial-adapte
(I do not own this article or the website.)

# Program Breakdown
A local *server* listening at `PORT_NUM = 5555` is created. This server waits until a *client* (more on clients later) successfully connects and provides it with the *COM Port number*. Once a client does that, the server **SET**s the **RTS** pin on the converter for **100ms** before **CL**ea**R**ing it. The DTR pin is **not used**.
Note: Some converters use negative logic for their control pins, i.e. a **SET** RTS pin means Voltage Low (LED off), and a **CLR** RTS pin means Voltage High (LED on). A simple test is to plug in the converter to a USB port in your pc and attach an LED to the RTS pin. If the LED stays lit all the time, this negative logic is used. 

# Usage
The previous section mentions a `client` without describing what it is. The advantage of creating a local server is that initiating a connection is simple. Go to your browser, and in the URL, type `localhost:5555?COM=42`. Here, `5555` is the `PORT_NUM` global variable in the source file, which is `5555` by default unless changed at compile time. The `42` refers to the COM port assigned to your Converter or any other device. This COM port varies and must be found so you can supply it to the HTTP request.

Example:- You find out your COM port is 12. You will type `localhost:5555?COM=12` in your browser URL bar.

If the request works, i.e. the program was running and a valid COM port number was given, an `200 OK` response is sent back to the browser. Otherwise, a `400 BAD REQUEST` response is sent back. Both of these will be visible as plain text in the browser window where you try to connect as a client.
