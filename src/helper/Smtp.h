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


#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QChar>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

class Smtp : public QObject {
Q_OBJECT

public:
	Smtp(const QString &host, const int port);
	virtual ~Smtp();
	void authLogin(const QString &username, const QString &password);
	void authPlain(const QString &username, const QString &password);
	bool send(const QString &from, const QString &to, const QString &subject, const QString &body);
	
signals:
	void status(const QString &);
	
private slots:
	void stateChanged(QTcpSocket::SocketState socketState);
	void errorReceived(QTcpSocket::SocketError socketError);
	void disconnected();
	void connected();
	void readyRead();
 
private:
	bool isConnected;
	bool _authLogin;
	QString username;
	QString password;
	QString host;
	int port;
	QString message;
	QTextStream *textStream;
	QTcpSocket *socket;
	QString from;
	QString rcpt;
	QString response;
	enum states { Init, Auth, User, Pass, Rcpt, Mail, Data, Body, Quit, Close };
	int state;
	QString DateHeader();
};

#endif // SMTP_H
