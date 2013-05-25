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

#ifndef SMTP_H
#define SMTP_H

#ifndef WIN32
#include <magic.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <QObject>
#include <QSslSocket>
#include <QString>
#include <QChar>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QPrinter>
#include <QList>
#include <QDateTime>
#include <QByteArray>
#include <QUuid>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QFileInfo>

class Smtp : public QObject {
Q_OBJECT

public:
	const static int SMTP_CHUNK_SIZE = 76;
	Smtp(const QString &host, const int port);
	virtual ~Smtp();
	void authLogin(const QString &username, const QString &password);
	void authPlain(const QString &username, const QString &password);
	void attach(const QString &file, const QString &name);
	void clearAttachments();
	void setTextMessage(const QString &body);
	void setHtmlMessage(const QString &body);
	bool send(const QString &from, const QString &to, const QString &subject);
	
	struct attachment_t {
		QString fileName;
		QString mimeType;
		QString data;
		QString contentDisposition;
		QString boundary;
	};
	
signals:
	void status(const QString &);
	
private slots:
	void stateChanged(QAbstractSocket::SocketState socketState);
	void errorReceived(QAbstractSocket::SocketError socketError);
	void disconnected();
	void connected();
	void readyRead();
 
private:
	bool isConnected;
	bool _authLogin;
	bool _useTLS;
	bool _useSSL;
	QString username;
	QString password;
	QString host;
	int port;
	QString message;
	QString textBody;
	QString htmlBody;
	QTextStream *textStream;
	QSslSocket *socket;
	QString from;
	QString rcpt;
	QString response;
	enum states { Init, Init_TLS, Auth, User, Pass, Rcpt, Mail, Data, Body, Quit, Close };
	int state;
	QList<attachment_t> attachments;
	int readyReadCount;
	
	QString dateHeader();
	QString generateBoundary();
	QString getMimeType(const QString &fileName);
	QString chuckSplit(const QString &data, bool wordwise = false, bool isHtml = false);
#ifdef SMTP_SAVE_DEBUG
	void saveEmailDebug(const QString &from, const QString &to, const QString &subject, const QString &message);
#endif
};

#endif // SMTP_H
