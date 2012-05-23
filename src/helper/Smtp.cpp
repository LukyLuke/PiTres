/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Lukas Zurschmiede <l.zurschmiede@delightsoftware.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "Smtp.h"

#include <cstdlib>
#include <ctime>
#include <QDateTime>
#include <QByteArray>
#include <QSettings>

Smtp::Smtp(const QString &host, const int port):QObject() {
	this->host = host;
	this->port = port;
	_authLogin = false;
}

Smtp::~Smtp() {
	delete textStream;
	delete socket;
}

bool Smtp::send(const QString & from, const QString & to, const QString & subject, const QString & body) {
	int timeout = 5000;
	
	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(socket, SIGNAL(error(QTcpSocket::SocketError)), this, SLOT(errorReceived(QTcpSocket::SocketError)));
	connect(socket, SIGNAL(stateChanged(QTcpSocket::SocketState)), this, SLOT(stateChanged(QTcpSocket::SocketState)));
	connect(socket, SIGNAL(disconnectedFromHost()), this, SLOT(disconnected()));
	
	message = DateHeader() + "\n";
	message.append("X-Accept-Language: en-us, en\n");
	message.append("MIME-Version: 1.0\n");
	message.append("To: " + to + "\n");
	message.append("From: " + from + "\n");
	message.append("Subject: " + subject + "\n");
	message.append("Content-Type: text/plain; charset=UTF8;\n");
	message.append("Content-transfer-encoding: 7BIT\n\n\n\n");
	message.append(body);
	message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
	message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));
	
	this->from = from;
	rcpt = to;
	state = Init;
	
	isConnected = false;
	socket->connectToHost(host, port);
	if (socket->waitForConnected(timeout)) {
		qDebug() << "Smtp: Connected on " << host << ":" << QString::number(port);
		if (socket->waitForReadyRead(timeout)) {
			qDebug() << "Smtp: emit from waitForReadyRead, connect can go on...";
			isConnected = true;
		}
	}
	textStream = new QTextStream(socket);
	
	return socket->waitForDisconnected(-1);
}

QString Smtp::DateHeader() {
	// mail rfc; Date format! http://www.faqs.org/rfcs/rfc788.html
	QDateTime currentTime = QDateTime::currentDateTime();
	QStringList RfcDays = QStringList() << "Mon" << "Tue" << "Wed" << "Thu" << "Fri" << "Sat" << "Sun";
	QStringList RfcMonths = QStringList() << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";
	
	return QString("Date: %1, %2 %3 %4 %5").arg(
		RfcDays.at(currentTime.date().dayOfWeek()-1),
		QString::number(currentTime.date().day()),
		RfcMonths.at(currentTime.date().month()-1),
		QString::number(currentTime.date().year()),
		currentTime.toString("hh:mm:ss")
	);
}

void Smtp::authLogin(const QString &username, const QString &password) {
	_authLogin = true;
	QByteArray ba;
	ba.append(username);
	this->username = ba.toBase64();
	ba.clear();
	ba.append(password);
	this->password = ba.toBase64();
}

void Smtp::authPlain(const QString &username, const QString &password) {
	_authLogin = false;
	this->username = "";
	QByteArray ba;
	ba.append(username);
	this->username.append(ba.toBase64());
	this->username.append(QChar::Null);
	this->username.append(ba.toBase64());
	this->username.append(QChar::Null);
	
	ba.clear();
	ba.append(password);
	this->username.append(ba.toBase64());
}

void Smtp::stateChanged(QTcpSocket::SocketState socketState) {
	qDebug() << "Smtp: State Changed:" << socketState;
}

void Smtp::errorReceived(QTcpSocket::SocketError socketError) {
	qDebug() << "Smtp: Socket-Error:" << socketError;
}

void Smtp::disconnected() {
	qDebug() << "Smtp: Disconnected";
	qDebug() << "Smtp: Error: "  << socket->errorString();
}

void Smtp::connected() {
	qDebug() << "Smtp: Connected";
}

void Smtp::readyRead() {
	QSettings settings;
	qDebug() << "Smtp: Ready for reading...";
	
	// Read Line-by-Line
	QString responseLine;
	do {
		responseLine = socket->readLine();
		response += responseLine;
	} while(socket->canReadLine() && responseLine[3] != ' '); // first 3 chars are the State-Number like 250
	responseLine.truncate(3); // remove everything after the State-Number
	
	// Close
	if (state == Close) {
		socket->disconnectFromHost();
		deleteLater();
		return;
	}
	
	// Mail-Body
	if (state == Body && responseLine[0] == '3') {
		*textStream << message << "\r\n.\r\n";
		textStream->flush();
		state = Quit;
		response = "";
		return;
	}
	
	// Every other Smtp-Code must be from 2xx
	if (responseLine[0] != '2' && state != User && state != Pass) {
		qDebug() << "Smtp: Unexpected reply from SMTP-Host: : \n" << response;
		state = Close;
		response = "";
		return;
	}
	
	switch (state) {
		// Initialize
		case Init:
			*textStream << "EHLO " << settings.value("smtp/ehlo_host", "nohost.local").toString() << "\r\n";
			textStream->flush();
			state = username.isEmpty() ? Mail : Auth;
			break;
		
		// Authentication header
		case Auth:
			if (_authLogin) {
				*textStream << "AUTH LOGIN\r\n";
				state = User;
			} else {
				*textStream << "AUTH PLAIN " + username + "\r\n";
				state = Mail;
			}
			textStream->flush();
			break;
		
		// Username for AUTH LOGIN
		case User:
			*textStream << username + "\r\n";
			textStream->flush();
			state = Pass;
			break;
		
		// Password for AUTH LOGIN
		case Pass:
			*textStream << password + "\r\n";
			textStream->flush();
			state = Mail;
			break;
			
		// EHLO-Response was OK, send from
		case Mail:
			*textStream << "MAIL FROM: " << from << "\r\n";
			textStream->flush();
			state = Rcpt;
			break;
			
		// MAIL-FROM-Response was OK, send rcpt to
		case Rcpt:
			*textStream << "RCPT TO: " << rcpt << "\r\n";
			textStream->flush();
			state = Data;
			break;
			
		// RCPT-TO-Response was OK, send Data
		case Data:
			*textStream << "DATA\r\n";
			textStream->flush();
			state = Body;
			break;
			
		// DATA-Response was OK, send mail
		case Quit:
			*textStream << "QUIT\r\n";
			textStream->flush();
			state = Close;
			emit status(tr("Message sent"));
			break;
	}
	response = "";
}

