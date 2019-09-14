# Programmer Name: Haya Ahmed
# Program Name: CS 372 Project 2, fclient.py
# Program Description: Client side of the ftp application, written in python
# Course Name: CS 372, Intro to Computer Networks
# Last Modified: August 15, 2019
# Resources used:
# Notes from 344 on Network Clients, by Benjamin Brewster
# Beej's Guide to Network Programming
# https: // stackoverflow.com / questions / 4204666 / how - to - list - files - in -a - directory - in -a - c - program
# https: // stackoverflow.com / questions / 9875735 / c - socket - programming - a - client - server - application - to - send - a - file

from socket import *
from sys import argv, exit, stdout
import os


##################################################
# Main Description: rec file
# Pre-conditions: host and port filename are obtained
# Post-conditions: rec and print directory
##################################################
def recieveFile(serverHost, dataSocket, filename, clientSocket):

    files = os.listdir(os.getcwd())
    fileFound = False

    for file in files:
        if file.find(filename) != -1:
            fileFound = True

    if fileFound:
        response = raw_input("File found. Would you like to make a copy? (y/n): ")
        if response == "y":
            newFile = "copy-of-" + filename
            f = open(newFile, "w+")  # Open a new file with the prepend "copy-"

            connectionSocket, addr = dataSocket.accept()
            print 'Recieving \"' + filename + '\" from ' + serverHost

            while True:
                file_data = connectionSocket.recv(256)
                if file_data.endswith(chr(1)):
                    f.write(file_data[:-1])
                    break
                f.write(file_data)

            print 'File transfer complete.\nFile saved as copy-of-' + filename
            f.close()  # Close the file
        else:
            print "File transfer aborted"
    else:
        message = clientSocket.recv(100)
        print message
        #print "File not found \n"


##################################################
# Main Description: rec directory
# Pre-conditions: host and port are obtained in main
# Post-conditions: rec and print directory
##################################################
def recieveDirectory(serverHost, dataSocket):
    connectionSocket, addr = dataSocket.accept()
    print'Recieving directory from ' + serverHost

    directoryData = connectionSocket.recv(2000)
    stdout.write(directoryData)



##################################################
# Main Description: sets up data connetion
# Pre-conditions: host and port are obtained in main
# Post-conditions: sets data connection
##################################################
def set_data_port(dataPort):
    dataSocket = socket(AF_INET, SOCK_STREAM)
    dataSocket.bind((gethostname(), dataPort))
    dataSocket.listen(1)
    return dataSocket


##################################################
# Main Description: makes request
# Pre-conditions: valid parameters are obtained
# Post-conditions: sends command
##################################################
def makeRequest(clientSocket, command_type, filename, dataPort):
    if command_type:  # -g == 1
        toSend = str(command_type) + "|" + str(dataPort) + "|" + filename
    else:
        toSend = str(command_type) + "|" + str(dataPort)
    clientSocket.send(toSend)


# /******************************************************
# Main Description: sets up socket connetion
# Pre-conditions: host and port are obtained in main
# Post-conditions: sets connection
# *******************************************************/
def setConnection(hostname, portnumber):
    clientSocket = socket(AF_INET, SOCK_STREAM)
    clientSocket.connect((hostname, portnumber))
    return clientSocket



def main():
    # Get command line arguments
    args_len = len(argv)
    if args_len < 5:
        print "python fclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND (-g <FILENAME> or -l)>"
        exit(1)
    else:
        serverHost = argv[1]
        serverPort = int(argv[2])
        command = argv[3]
        if args_len == 6 and command == '-g':
            command_type = 1
            filename = argv[4]
            dataPort = int(argv[5])
        elif args_len == 5 and command == '-l':
            command_type = 0
            filename = ''
            dataPort = int(argv[4])
        else:
            print 'python fclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND (-g <FILENAME> or -l)>>'
            exit(1)

    clientSocket = setConnection(serverHost, serverPort)  # Set up connection with server

    dataSocket = socket(AF_INET, SOCK_STREAM)
    dataSocket.bind((gethostname(), dataPort))
    dataSocket.listen(1)

    makeRequest(clientSocket, command_type, filename, dataPort)  # Send command to server

    if command_type:  # If we're getting a file
        recieveFile(serverHost, dataSocket, filename, clientSocket)
    else:
        recieveDirectory(serverHost, dataSocket)

    # Close sockets
    dataSocket.close()
    clientSocket.close()

if __name__ == '__main__':
    main()