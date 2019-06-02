// Copyright (c) 2013-2015 The Feathercoin developers
#ifndef MESSAGEPAGE_H
#define MESSAGEPAGE_H

#include "guiutil.h"

#include <QWidget>

namespace Ui {
    class MessagePage;
}
class WalletModel;
class ClientModel;

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class MessagePage : public /*QWidget*/QDialog
{
    Q_OBJECT

public:
    explicit MessagePage(QWidget *parent = 0);
    ~MessagePage();

    void setModel(WalletModel *model);
    void setClientModel(ClientModel *model);

    void setAddress_SM(const QString &address);
    void setAddress_VM(const QString &address);
    void showTab_SM(bool fShow);
    void showTab_VM(bool fShow);

    void setAddress_ED(const QString &address);
    void setAddress_DD(const QString &address);

    void showTab_ED(bool fShow);
    void showTab_DD(bool fShow);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::MessagePage *ui;
    WalletModel *model;
    ClientModel *clientModel;

private slots:
    /* sign message */
    void on_addressBookButton_SM_clicked();
    void on_pasteButton_SM_clicked();
    void on_signMessageButton_SM_clicked();
    void on_copySignatureButton_SM_clicked();
    void on_clearButton_SM_clicked();
    /* verify message */
    void on_addressBookButton_VM_clicked();
    void on_verifyMessageButton_VM_clicked();
    void on_clearButton_VM_clicked();

    /* encrypt message */
    void on_addressBookButton_ED_clicked();
    void on_pasteButton_ED_clicked();
    void on_encryptMessageButton_ED_clicked();
    void on_copyEncryptedDataButton_ED_clicked();
    void on_clearButton_ED_clicked();
    /* decrypt message */
    void on_addressBookButton_DD_clicked();
    void on_decryptMessageButton_DD_clicked();
    void on_clearButton_DD_clicked();

signals:
    //void doubleClicked(const QModelIndex&);

    /**  Fired when a message should be reported to the user */
    void message(const QString &title, const QString &message, unsigned int style);


public slots:

};

#endif // MESSAGEPAGE_H
