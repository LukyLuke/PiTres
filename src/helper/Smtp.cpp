/*
	A simple SMTP-Class to send emails with attachmets.
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

/**
 * For debuging your program, you can define two constants:
 * 
 *   SMTP_DEBUG
 *     If set, no mails are sent out.
 * 
 *   SMTP_SAVE_DEBUG
 *     If set, all mails are saved as .eml in the "debug" subfolder of the running path.
 * 
 * If you need to check the form of the emails before the golive, enable both.
 * If you are developing parts witch sends emails. just enable SMTP_DEBUG to not send them.
 */

#include "Smtp.h"

Smtp::Smtp(const QString &host, const int port) : QObject() {
	QSettings settings;
	
	this->host = host;
	this->port = port;
	_authLogin = FALSE;
	_useTLS = settings.value("smtp/use_tls", FALSE).toBool();
	_useSSL = settings.value("smtp/use_ssl", FALSE).toBool();
	readyReadCount = 0;
}

Smtp::~Smtp() {
	delete textStream;
	delete socket;
}

bool Smtp::send(const QString &from, const QString &to, const QString &subject) {
	int timeout = 5000;
	
	socket = new QSslSocket(this);
	if (_useSSL) {
		connect(socket, SIGNAL(encrypted()), this, SLOT(connected()));
	} else {
		connect(socket, SIGNAL(connected()), this, SLOT(connected()));
	}
	connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorReceived(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
	connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
	
	QString msgBoundary = generateBoundary();
	QString bodyBoundary = generateBoundary();
	
	QString msgTextBody = textBody.replace("\n", "\r\n");
	msgTextBody.replace("\r\n.\r\n", "\r\n..\r\n");
	msgTextBody = chuckSplit(msgTextBody, TRUE);
	
	QString msgHtmlBody = htmlBody.replace("\n", "\r\n");
	msgHtmlBody.replace("\r\n.\r\n", "\r\n..\r\n");
	
	// BUG: swisscom webmail cannot handle this and throws an exception.
	//      this error is currently discussed by them, maybe they can solve it :o)
	if (to.contains(QRegExp("(swisscom|bluewin|bluemail)\.(ch|com)"))) {
		msgHtmlBody.replace(QRegExp("\"data:image[^\"]+\""), "");
	}
	
	message = dateHeader() + "\r\n";
	message.append("MIME-Version: 1.0\r\n");
	message.append("To: " + to + "\r\n");
	message.append("From: " + from + "\r\n");
	message.append("Subject: " + subject + "\r\n");
	message.append("Content-Type: multipart/mixed;\r\n\tboundary=\""+msgBoundary+"\"\r\n");
	message.append("This is a multi-part message in MIME format.\r\n\r\n");
	message.append("--" + msgBoundary);
	message.append("\r\nContent-Type: multipart/alternative;\r\n\tboundary=\""+bodyBoundary+"\"\r\n\r\n");
	message.append("--" + bodyBoundary + "\r\n");
	message.append("Content-Type: text/plain; charset=\"utf-8\"\r\n");
	message.append("Content-Transfer-Encoding: 8Bit\r\n\r\n");
	message.append(msgTextBody);
	message.append("\r\n\r\n--" + bodyBoundary + "\r\n");
	message.append("Content-Type: text/html; charset=\"utf-8\"\r\n");
	message.append("Content-Transfer-Encoding: 8Bit\r\n\r\n");
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
	
	isConnected = FALSE;
	textStream = new QTextStream(socket);
	
#ifdef SMTP_SAVE_DEBUG
	saveEmailDebug(from, to, subject, message);
#endif
	
#ifdef SMTP_DEBUG
	qDebug() << "SMTP_DEBUG: I do not send an Email! This is a DEBUG-BUILD without sending Emails!";
	qDebug() << "SMTP_DEBUG: Remove the 'SMTP_DEBUG' in PiTres.pro and recompile with 'qmake && make'...";
	
	return false;
#else
	if (_useSSL) {
		socket->connectToHostEncrypted(host, port);
	} else {
		socket->connectToHost(host, port);
	}
	if ((!_useSSL && socket->waitForConnected(timeout)) || (_useSSL && socket->waitForEncrypted(timeout))) {
		qDebug() << "Smtp: Connected on " << host << ":" << QString::number(port);
		if (socket->waitForReadyRead(timeout)) {
			qDebug() << "Smtp: emit from waitForReadyRead, connect can go on...";
			readyReadCount = 1;
			readyRead();
		}
	} else {
		qWarning() << "Smtp: Connection-Error: " << socket->errorString();
	}
	
	return socket->waitForDisconnected(-1);
#endif
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
	if (socket->error() != QAbstractSocket::UnknownSocketError) {
		qWarning() << "Smtp: Error: "  << socket->errorString();
	}
}

void Smtp::connected() {
	isConnected = TRUE;
	qDebug() << "Smtp: Connected";
}

void Smtp::readyRead() {
#ifdef SHOW_SMTP_DEBUG_LOG
	qDebug() << "Smtp: readyRead emitted";
#endif
	if (!isConnected || readyReadCount == 0) {
		return;
	}
	QSettings settings;

	// Read Line-by-Line
	QString responseLine;
	do {
		responseLine = socket->readLine();
		response += responseLine;
	} while(socket->canReadLine() && responseLine[3] != ' '); // first 3 chars are the State-Number like 250
	responseLine.truncate(3); // remove everything after the State-Number

#ifdef SHOW_SMTP_DEBUG_LOG
	qDebug() << "Smtp: response(" << readyReadCount << "): " << response.trimmed();
	qDebug() << "Smtp: last code(" << readyReadCount << "): " << responseLine;
	readyReadCount++;
#endif

	// Close
	if (state == Close) {
#ifdef SHOW_SMTP_DEBUG_LOG
		qDebug() << "Smtp: State close";
#endif
		socket->disconnectFromHost();
		deleteLater();
		return;
	}
	
	// Check for forced TLS by us.
	if (_useTLS && !socket->isEncrypted()) {
		// After the first EHLO, we must find a STARTTLS in the response.
		if ((state == Mail || state == Auth) && response.contains("STARTTLS")) {
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: STARTTLS";
#endif
			*textStream << "STARTTLS\r\n";
			textStream->flush();
			state = Init_TLS;
			
		// If we get the right answer after STARTTLS, enforce the encryption.
		} else if ((state == Init_TLS) && (responseLine == "220")) {
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Initiate client encryption for TLS";
#endif
			socket->startClientEncryption();
			state = Init;
		}
	}
	
	// Mail-Body
	if (state == Body && responseLine[0] == '3') {
#ifdef SHOW_SMTP_DEBUG_LOG
		qDebug() << "Smtp: Send email data";
#endif
		*textStream << message << "\r\n.\r\n";
		textStream->flush();
		state = Quit;
		response = "";
		return;
	}
	
	// Every other Smtp-Code must be from 2xx
	if (responseLine[0] != '2' && state != User && state != Pass) {
		qDebug() << "Smtp: Unexpected reply from SMTP-Host: \n" << response.trimmed();
#ifdef SHOW_SMTP_DEBUG_LOG
		qDebug() << "Smtp: Send QUIT";
#endif
		*textStream << "QUIT\r\n";
		textStream->flush();
		state = Close;
		response = "";
		return;
	}
	
	switch (state) {
		// Initialize
		case Init:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send EHLO " << settings.value("smtp/ehlo_host", "nohost.local").toString();
#endif
			*textStream << "EHLO " << settings.value("smtp/ehlo_host", "nohost.local").toString() << "\r\n";
			textStream->flush();
			state = username.isEmpty() ? Mail : Auth;
			break;
		
		// Authentication header
		case Auth:
			if (_authLogin) {
#ifdef SHOW_SMTP_DEBUG_LOG
				qDebug() << "Smtp: Send AUTH LOGIN";
#endif
				*textStream << "AUTH LOGIN\r\n";
				state = User;
			} else {
#ifdef SHOW_SMTP_DEBUG_LOG
				qDebug() << "Smtp: AUTH PLAIN " << username;
#endif
				*textStream << "AUTH PLAIN " + username + "\r\n";
				state = Mail;
			}
			textStream->flush();
			break;
		
		// Username for AUTH LOGIN
		case User:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send username " << username;
#endif
			*textStream << username + "\r\n";
			textStream->flush();
			state = Pass;
			break;
		
		// Password for AUTH LOGIN
		case Pass:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send password " << password;
#endif
			*textStream << password + "\r\n";
			textStream->flush();
			state = Mail;
			break;
			
		// EHLO-Response was OK, send from
		case Mail:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send MAIL FROM: " << from;
#endif
			*textStream << "MAIL FROM: " << from << "\r\n";
			textStream->flush();
			state = Rcpt;
			break;
			
		// MAIL-FROM-Response was OK, send rcpt to
		case Rcpt:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send RCPT TO: " << rcpt;
#endif
			*textStream << "RCPT TO: " << rcpt << "\r\n";
			textStream->flush();
			state = Data;
			break;
			
		// RCPT-TO-Response was OK, send Data
		case Data:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send DATA";
#endif
			*textStream << "DATA\r\n";
			textStream->flush();
			state = Body;
			break;
			
		// DATA-Response was OK, send mail
		case Quit:
#ifdef SHOW_SMTP_DEBUG_LOG
			qDebug() << "Smtp: Send QUIT";
#endif
			*textStream << "QUIT\r\n";
			textStream->flush();
			state = Close;
			emit status(tr("Message sent"));
			break;
	}
	response = "";
}

void Smtp::setTextMessage(const QString &body) {
	textBody = body.toUtf8();
}

void Smtp::setHtmlMessage(const QString &body) {
	htmlBody = body.toUtf8();
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

#ifdef SMTP_SAVE_DEBUG
void Smtp::saveEmailDebug(const QString &from, const QString &to, const QString &subject, const QString &message) {
	QTextStream out;
	QDir tmp(QDir::currentPath().append("/debug/").append(QString(to).replace('@', "-at-")));
	qDebug() << "Save EMail for " << to << " into " << tmp.absolutePath();
	tmp.mkpath(tmp.absolutePath());
	
	// Writing information file
	QFile info(tmp.absolutePath().append("/info.txt"));
	if (!info.open(QIODevice::WriteOnly | QFile::Truncate | QFile::Text)) {
		qDebug() << "Unable to open SMTP-Debug file: " << info.fileName();
	} else {
		out.setDevice(&info);
		out << QString("From: ").append(from).append("\n");
		out << QString("To: ").append(to).append("\n");
		out << QString("Subject: ").append(subject).append("\n");
		out << dateHeader().append("\n");
		info.close();
	}
	
	// Writing the messsage
	QFile msg(tmp.absolutePath().append("/message.eml"));
	if (!msg.open(QIODevice::WriteOnly | QFile::Truncate | QFile::Text)) {
		qDebug() << "Unable to open SMTP-Debug file: " << msg.fileName();
	} else {
		out.setDevice(&msg);
		out << message;
		msg.close();
	}
}
#endif

