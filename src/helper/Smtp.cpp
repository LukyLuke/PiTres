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
#include <QSettings>

Smtp::Smtp(const QString & from, const QString & to, const QString & subject, const QString & body) {
	QSettings settings;
	socket = new QTcpSocket(this);
 
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(socket, SIGNAL(error(SocketError)), this, SLOT(errorReceived(SocketError)));
	connect(socket, SIGNAL(stateChanged(SocketState)), this, SLOT(stateChanged(SocketState)));
	connect(socket, SIGNAL(disconnectedFromHost()), this, SLOT(disconnected()));
	
	message = DateHeader() + "\n";
	message.append("To: " + to + "\n");
	message.append("From: " + from + "\n");
	message.append("Subject: " + subject + "\n");
	message.append(body);
	message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
	message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));
	
	this->from = from;
	rcpt = to;
	state = Init;
	
	socket->connectToHost(settings.value("smtp/server", "localhost").toString(), settings.value("smtp/port", 25).toInt());
	if (socket->waitForConnected(30000)) {
		qDebug() << "Smtp: Connected";
	}
	
	textStream = new QTextStream(socket); 
}

Smtp::~Smtp() {
	delete textStream;
	delete socket;
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
	if (responseLine[0] != '2') {
		qDebug() << "Smtp: Unexpected reply from SMTP-Host: : \n" << response;
		state = Close;
		response = "";
		return;
	}
	
	switch (state) {
		// Initialize
		case Init:
			*textStream << "EHLO " << settings.value("smtp/ehlo_host", "here.local").toString() << "\r\n";
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

// mail rfc; Date format! http://www.faqs.org/rfcs/rfc788.html
QString Smtp::DateHeader() {
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

