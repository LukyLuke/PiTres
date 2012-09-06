/*
	A simple SMTP-Class to send EMails with Attachmets
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

#ifndef WIN32
#include <magic.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <QDateTime>
#include <QByteArray>
#include <QUuid>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QFileInfo>

Smtp::Smtp(const QString &host, const int port):QObject() {
	this->host = host;
	this->port = port;
	_authLogin = false;
}

Smtp::~Smtp() {
	delete textStream;
	delete socket;
}

bool Smtp::send(const QString & from, const QString & to, const QString & subject) {
	int timeout = 5000;
	
	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(connected()), this, SLOT(connected()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorReceived(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
	connect(socket, SIGNAL(disconnectedFromHost()), this, SLOT(disconnected()));
	
	QString msgBoundary = generateBoundary();
	QString bodyBoundary = generateBoundary();
	
	QString msgTextBody = textBody.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
	msgTextBody.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));
	msgTextBody = chuckSplit(msgTextBody, true);
	
	QString msgHtmlBody = htmlBody.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
	msgHtmlBody.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));
	//msgHtmlBody = chuckSplit(msgHtmlBody, false, true);
	
	message = dateHeader() + "\r\n";
	message.append("MIME-Version: 1.0\r\n");
	message.append("To: " + to + "\r\n");
	message.append("From: " + from + "\r\n");
	message.append("Subject: " + subject + "\r\n");
	message.append("Content-Type: multipart/mixed;\r\n\tboundary=\""+msgBoundary+"\"\r\n");
	message.append("Content-transfer-encoding: 7BIT\r\n\r\n");
	message.append("This is a multi-part message in MIME format.\r\n\r\n");
	message.append("--" + msgBoundary);
	message.append("\r\nContent-Type: multipart/alternative;\r\n\tboundary=\""+bodyBoundary+"\"\r\n\r\n");
	message.append("--" + bodyBoundary + "\r\n");
	message.append("Content-Type: text/plain; charset=\"utf-8\"\r\n");
	message.append("Content-Transfer-Encoding: quoted-printable\r\n\r\n");
	message.append(msgTextBody);
	message.append("\r\n\r\n--" + bodyBoundary + "\r\n");
	message.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
	message.append("Content-Transfer-Encoding: quoted-printable\r\n\r\n");
	message.append(msgHtmlBody + "\r\n");
	message.append("--" + bodyBoundary + "--\r\n\r\n");
	
	// Attachments
	QString boundary;
	for (int i = 0; i < attachments.length(); i++) {
		attachment_t a = attachments.at(i);
		message.append("--" + msgBoundary + "\r\n");
		message.append("Content-Type: " + a.mimeType + ";\r\n\tname=\"" + a.fileName + "\"\r\n");
		message.append("Content-Transfer-Encoding: base64\r\n");
		message.append("Content-Disposition: " + a.contentDisposition + ";\r\n\tfilename=\"" + a.fileName + "\"\r\n\r\n");
		message.append(chuckSplit(a.data));
		message.append("\r\n--" + msgBoundary + "--\r\n\r\n");
	}
	
	this->from = from;
	rcpt = to;
	state = Init;
	
	isConnected = false;
	textStream = new QTextStream(socket);
	socket->connectToHost(host, port);
	if (socket->waitForConnected(timeout)) {
		qDebug() << "Smtp: Connected on " << host << ":" << QString::number(port);
		if (socket->waitForReadyRead(timeout)) {
			qDebug() << "Smtp: emit from waitForReadyRead, connect can go on...";
			isConnected = true;
		}
	}
	
	return socket->waitForDisconnected(-1);
}

QString Smtp::chuckSplit(const QString &data, bool wordwise, bool isHtml) {
	QStringList list;
	
	if (data.size() == 0) {
		list.append(" ");
		
	} else if (wordwise) {
		// Line by Line
		QStringList li = data.split(QRegExp("\r?\n"), QString::KeepEmptyParts);
		for (int j = 0; j < li.size(); j++) {
			list.append(li.at(j));
		}
	} else {
		int i = 0, max = data.size() + SMTP_CHUNK_SIZE;
		while (i < max) {
			list.append(data.mid(i, SMTP_CHUNK_SIZE));
			i += SMTP_CHUNK_SIZE;
		}
	}
	return list.join(QString(isHtml ? "=" : "").append("\r\n"));
}

QString Smtp::dateHeader() {
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

void Smtp::stateChanged(QAbstractSocket::SocketState socketState) {
	qDebug() << "Smtp: State Changed:" << socketState;
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError) {
	qWarning() << "Smtp: Socket-Error:" << socketError;
}

void Smtp::disconnected() {
	qWarning() << "Smtp: Disconnected";
	qWarning() << "Smtp: Error: "  << socket->errorString();
}

void Smtp::connected() {
	qDebug() << "Smtp: Connected";
}

void Smtp::readyRead() {
	QSettings settings;
	
	// Read Line-by-Line
	QString responseLine;
	do {
		responseLine = socket->readLine();
		response += responseLine;
	} while(socket->canReadLine() && responseLine[3] != ' '); // first 3 chars are the State-Number like 250
	responseLine.truncate(3); // remove everything after the State-Number
	qDebug() << "Smtp: " << response.trimmed();
	
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
		*textStream << "QUIT\r\n";
		textStream->flush();
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

void Smtp::setTextMessage(const QString &body) {
	textBody = body;
}

void Smtp::setHtmlMessage(const QString &body) {
	htmlBody = body;
	if (htmlBody.indexOf("<html", 0, Qt::CaseInsensitive) < 0) {
		htmlBody.prepend("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><style></style></head><body>");
		htmlBody.append("</body></html>");
	}
}

void Smtp::attach(const QString &file, const QString &name) {
	QFile f(file);
	f.open(QIODevice::ReadOnly);
	QFileInfo fi(file);
	qDebug() << "Smtp: Attach File: " << file;
	
	attachment_t a;
	a.boundary = generateBoundary();
	a.fileName = name.isEmpty() ? fi.fileName() : name;
	a.contentDisposition = "attachment; filename=\"" + name + "\"";
	a.mimeType = getMimeType(file);
	a.data = f.readAll().toBase64();
	attachments.append(a);
	f.close();
}

void Smtp::clearAttachments() {
	attachments.clear();
}

QString Smtp::generateBoundary() {
	QString s = QUuid::createUuid().toString().replace(QRegExp("\\{\\}"), "");
	s.prepend("----=_NextPart_");
	return s;
}

QString Smtp::getMimeType(const QString &fileName) {
	// Thanks to Sorokin Vasiliy
	// http://va-sorokin.blogspot.com/2011/03/how-to-get-mime-type-on-nix-system.html
	// ToDo: Windows-Version
	// ToDo: Mac-Version
	QString result("applicatin/octet-stream");
#ifndef WIN32
	magic_t magicMimePredictor;
	
	magicMimePredictor = magic_open(MAGIC_MIME_TYPE);
	if (!magicMimePredictor) {
		qDebug() << "Smtp: LibMagic failed initialize...";
		
	} else {
		// Loading the DB returns '0'
		if (magic_load(magicMimePredictor, 0)) {
			qDebug() << "Smtp: LibMagic failed to load the Mime-Database";
			
		} else {
			char *file = fileName.toAscii().data();
			const char *mime;
			mime = magic_file(magicMimePredictor, file);
			result = QString(mime);
		}
		magic_close(magicMimePredictor);
	}
	qDebug() << "Smtp: LibMagic found Mimetype '" << result << "' for file " << fileName;
#endif
	return result;
}
