#ifndef Data_Section_H
#define Data_Section_H

#include <QObject>
#include <QString>
#include <QList>
#include <QSqlDatabase>

class Section : public QObject {
Q_OBJECT
Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
Q_PROPERTY(float amount READ amount WRITE setAmount NOTIFY amountChanged)
Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
Q_PROPERTY(QString account READ account WRITE setAccount NOTIFY accountChanged)
Q_PROPERTY(int treasurer READ treasurer WRITE setTreasurer NOTIFY treasurerChanged)

Q_ENUM(AmountType)

public:
	Section(QObject* parent = 0);
	Section(const QString name, QObject* parent = 0);
	virtual ~Section();
	void load(const QString name);
	void save();
	QList<Section *> children();
	Section *parent();
	static void createTables();
	
	// enums
	enum AmountType { AmountPercent=0, AmountMoney=1 };
	
	// Setter
	void setName(QString name);
	void setAmount(float amount);
	void setDescription(QString description);
	void setAddress(QString address);
	void setAccount(QString account);
	void setTreasurer(int treasurer);
	
	// Getter
	bool loaded() { return _loaded; };
	QString name() const { return s_name; }
	float amount() const { return f_amount; }
	QString description() const { return s_description; }
	QString address() const { return s_address; }
	QString account() const { return s_account; }
	int treasurer() const { return i_treasurer; }
	
signals:
	void nameChanged(QString);
	void amountChanged(float);
	void descriptionChanged(QString);
	void addressChanged(QString);
	void accountChanged(QString);
	void treasurerChanged(int);
	
private:
	QSqlDatabase db;
	bool _loaded;
	QString s_parent;
	QString s_name;
	float f_amount;
	QString s_description;
	QString s_address;
	QString s_account;
	int i_treasurer;
	
	void clear();
};

#endif //Data_Section_H
