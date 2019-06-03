#include "messagepage.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "addressbookpage.h"
#include "guiutil.h"
#include "walletmodel.h"
#include "ui_messagepage.h"
#include "base58.h"
#include "init.h"
#include "util.h"
#include "wallet.h"

#include "ui_interface.h"

#include <QMessageBox>
#include <QLabel>
#include <QClipboard>

MessagePage::MessagePage(QWidget *parent) :
    QDialog/*QWidget*/(parent),
    ui(new Ui::MessagePage),
    model(0),
    clientModel(0)

{
    ui->setupUi(this);

    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addressIn_SM->setPlaceholderText(tr("Enter a Fuguecoin address (e.g. FEyaPwV9xqRkCapaZSfJtrWhkhtyk9ohW1)"));
    ui->signatureOut_SM->setPlaceholderText(tr("Click \"Sign Message\" to generate signature"));
    ui->addressIn_VM->setPlaceholderText(tr("Enter a Fuguecoin address (e.g. FEyaPwV9xqRkCapaZSfJtrWhkhtyk9ohW1)"));
    ui->signatureIn_VM->setPlaceholderText(tr("Enter Fuguecoin signature"));
    ui->signatureOut_ED->setPlaceholderText(tr("Click \"Encrypt Message\" to encrypt the message"));
    ui->signatureIn_DD->setPlaceholderText(tr("Click \"Decrypt Message\" to decrypt the message"));
    ui->addressIn_ED->setPlaceholderText(tr("Enter a Fuguecoin address"));
    ui->addressIn_DD->setPlaceholderText(tr("Enter a Fuguecoin address"));

    GUIUtil::setupAddressWidget(ui->addressIn_SM, this);
    GUIUtil::setupAddressWidget(ui->addressIn_VM, this);
    GUIUtil::setupAddressWidget(ui->addressIn_ED, this);
    GUIUtil::setupAddressWidget(ui->addressIn_DD, this);

    ui->addressIn_SM->installEventFilter(this);
    ui->messageIn_SM->installEventFilter(this);
    ui->signatureOut_SM->installEventFilter(this);
    ui->addressIn_VM->installEventFilter(this);
    ui->messageIn_VM->installEventFilter(this);
    ui->signatureIn_VM->installEventFilter(this);

    ui->signatureOut_SM->setFont(GUIUtil::bitcoinAddressFont());
    ui->signatureIn_VM->setFont(GUIUtil::bitcoinAddressFont());
    ui->addressIn_ED->installEventFilter(this);
    ui->messageIn_ED->installEventFilter(this);
    ui->signatureOut_ED->installEventFilter(this);
    ui->addressIn_DD->installEventFilter(this);
    ui->messageIn_DD->installEventFilter(this);
    ui->signatureIn_DD->installEventFilter(this);

    ui->signatureOut_ED->setFont(GUIUtil::bitcoinAddressFont());
    ui->signatureIn_DD->setFont(GUIUtil::bitcoinAddressFont());
}

MessagePage::~MessagePage()
{
    delete ui;
}

void MessagePage::setModel(WalletModel *model)
{
    this->model = model;
    if(model)
    {}
}

void MessagePage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {}
}

void MessagePage::setAddress_SM(const QString &address)
{
    ui->addressIn_SM->setText(address);
    ui->messageIn_SM->setFocus();
}

void MessagePage::setAddress_VM(const QString &address)
{
    ui->addressIn_VM->setText(address);
    ui->messageIn_VM->setFocus();
}

void MessagePage::setAddress_ED(const QString &address)
{
    ui->addressIn_ED->setText(address);
    ui->messageIn_ED->setFocus();
}

void MessagePage::setAddress_DD(const QString &address)
{
    ui->addressIn_DD->setText(address);
    ui->messageIn_DD->setFocus();
}

void MessagePage::showTab_SM(bool fShow)
{
    ui->tabWidget->setCurrentIndex(0);

    if (fShow)
        this->show();
}

void MessagePage::showTab_VM(bool fShow)
{
    ui->tabWidget->setCurrentIndex(1);
    if (fShow)
        this->show();
}

void MessagePage::showTab_ED(bool fShow)
{
    ui->tabWidget->setCurrentIndex(2);
    if (fShow)
        this->show();
}

void MessagePage::showTab_DD(bool fShow)
{
    ui->tabWidget->setCurrentIndex(3);
    if (fShow)
        this->show();
}

void MessagePage::on_addressBookButton_SM_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_SM(dlg.getReturnValue());
        }
    }
}

void MessagePage::on_pasteButton_SM_clicked()
{
    setAddress_SM(QApplication::clipboard()->text());
}

void MessagePage::on_signMessageButton_SM_clicked()
{
    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */
    ui->signatureOut_SM->clear();

    CBitcoinAddress addr(ui->addressIn_SM->text().toStdString());
    if (!addr.IsValid())
    {
        ui->addressIn_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_SM->setValid(false);
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid())
    {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(tr("Private key for the entered address is not available."));
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_SM->document()->toPlainText().toStdString();

    std::vector<unsigned char> vchSig;
    if (!key.SignCompact(Hashfugue(ss.begin(), ss.end()), vchSig))
    {
        ui->statusLabel_SM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signing failed.") + QString("</nobr>"));
        return;
    }

    ui->statusLabel_SM->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_SM->setText(QString("<nobr>") + tr("Message signed.") + QString("</nobr>"));

    ui->signatureOut_SM->setText(QString::fromStdString(EncodeBase64(&vchSig[0], vchSig.size())));
}

void MessagePage::on_copySignatureButton_SM_clicked()
{
    QApplication::clipboard()->setText(ui->signatureOut_SM->text());
}

void MessagePage::on_clearButton_SM_clicked()
{
    ui->addressIn_SM->clear();
    ui->messageIn_SM->clear();
    ui->signatureOut_SM->clear();
    ui->statusLabel_SM->clear();

    ui->addressIn_SM->setFocus();
}

void MessagePage::on_addressBookButton_VM_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_VM(dlg.getReturnValue());
        }
    }
}

void MessagePage::on_verifyMessageButton_VM_clicked()
{
    CBitcoinAddress addr(ui->addressIn_VM->text().toStdString());
    if (!addr.IsValid())
    {
        ui->addressIn_VM->setValid(false);
        ui->statusLabel_VM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_VM->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_VM->setValid(false);
        ui->statusLabel_VM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_VM->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    bool fInvalid = false;
    std::vector<unsigned char> vchSig = DecodeBase64(ui->signatureIn_VM->text().toStdString().c_str(), &fInvalid);

    if (fInvalid)
    {
        ui->signatureIn_VM->setValid(false);
        ui->statusLabel_VM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_VM->setText(tr("The signature could not be decoded.") + QString(" ") + tr("Please check the signature and try again."));
        return;
    }

    CDataStream ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << ui->messageIn_VM->document()->toPlainText().toStdString();

    CKey key;
    if (!key.SetCompactSignature(Hashfugue(ss.begin(), ss.end()), vchSig))
    {
        ui->signatureIn_VM->setValid(false);
        ui->statusLabel_VM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_VM->setText(tr("The signature did not match the message digest.") + QString(" ") + tr("Please check the signature and try again."));
        return;
    }

    if (!(CBitcoinAddress(key.GetPubKey().GetID()) == addr))
    {
        ui->statusLabel_VM->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_VM->setText(QString("<nobr>") + tr("Message verification failed.") + QString("</nobr>"));
        return;
    }

    ui->statusLabel_VM->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_VM->setText(QString("<nobr>") + tr("Message verified.") + QString("</nobr>"));
}

void MessagePage::on_clearButton_VM_clicked()
{
    ui->addressIn_VM->clear();
    ui->signatureIn_VM->clear();
    ui->messageIn_VM->clear();
    ui->statusLabel_VM->clear();

    ui->addressIn_VM->setFocus();
}

void MessagePage::on_addressBookButton_ED_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::ReceivingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_ED(dlg.getReturnValue());
        }
    }
}

void MessagePage::on_pasteButton_ED_clicked()
{
    setAddress_ED(QApplication::clipboard()->text());
}

void MessagePage::on_encryptMessageButton_ED_clicked()
{
    if (!model)
        return;

    /* Clear old signature to ensure users don't get confused on error with an old signature displayed */
    ui->signatureOut_ED->clear();

    CBitcoinAddress addr(ui->addressIn_ED->text().toStdString());
    if (!addr.IsValid())
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_ED->setValid(false);
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    WalletModel::UnlockContext ctx(model->requestUnlock());
    if (!ctx.isValid())
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Wallet unlock was cancelled."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Private key for the entered address is not available."));
        return;
    }

    // void CKey::EncryptData(const std::vector<unsigned char> &plaindata, std::vector<unsigned char> &encdata) {
    std::vector<unsigned char> vchEncrypted;
    std::string strData = ui->messageIn_ED->document()->toPlainText().toStdString();
    key.EncryptData(std::vector<unsigned char>(strData.begin(), strData.end()), vchEncrypted);
    QString encrypted_message = QString::fromStdString(EncodeBase58Check(vchEncrypted));

    ui->messageIn_ED->setPlainText(encrypted_message);
    //ui->messageIn_ED->setReadOnly();

    ui->statusLabel_ED->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_ED->setText(QString("<nobr>") + tr("Message encrypted.") + QString("</nobr>"));
    ui->signatureOut_ED->setText(encrypted_message);
}

void MessagePage::on_copyEncryptedDataButton_ED_clicked()
{
    QApplication::clipboard()->setText(ui->signatureOut_ED->text());
}

void MessagePage::on_clearButton_ED_clicked()
{
    ui->addressIn_ED->clear();
    ui->messageIn_ED->clear();
    ui->signatureOut_ED->clear();
    ui->statusLabel_ED->clear();

    ui->addressIn_ED->setFocus();
}

void MessagePage::on_addressBookButton_DD_clicked()
{
    if (model && model->getAddressTableModel())
    {
        AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
        dlg.setModel(model->getAddressTableModel());
        if (dlg.exec())
        {
            setAddress_DD(dlg.getReturnValue());
        }
    }
}

void MessagePage::on_decryptMessageButton_DD_clicked()
{
    CBitcoinAddress addr(ui->addressIn_DD->text().toStdString());
    if (!addr.IsValid())
    {
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The entered address is invalid.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }
    CKeyID keyID;
    if (!addr.GetKeyID(keyID))
    {
        ui->addressIn_DD->setValid(false);
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The entered address does not refer to a key.") + QString(" ") + tr("Please check the address and try again."));
        return;
    }

    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
    {
        ui->statusLabel_ED->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_ED->setText(tr("Private key for the entered address is not available."));
        return;
    }

    std::string strData = ui->messageIn_ED->document()->toPlainText().toStdString();
    std::vector<unsigned char> vchEncrypted;
    if(!DecodeBase58Check(strData, vchEncrypted)) {
        ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel_DD->setText(tr("The message could not be decrypted.") + QString(" ") + tr("Please check the encrypted data and try again."));
        return;
    };

    std::vector<unsigned char> vchDecrypted;
    key.DecryptData(vchEncrypted, vchDecrypted);
    QString message = QString::fromStdString(std::string((const char *) &vchDecrypted[0], vchDecrypted.size()));
    ui->signatureIn_DD->setText(message);

    // CPubKey pubkey;
    // if (!pubkey.RecoverCompact(Hash(ss.begin(), ss.end()), vchSig))
    // {
    //     ui->signatureIn_DD->setValid(false);
    //     ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
    //     ui->statusLabel_DD->setText(tr("The address did not decrypt a message.") + QString(" ") + tr("Please check the encrypted data and try again."));
    //     return;
    // }

    // if (!(CBitcoinAddress(pubkey.GetID()) == addr))
    // {
    //     ui->statusLabel_DD->setStyleSheet("QLabel { color: red; }");
    //     ui->statusLabel_DD->setText(QString("<nobr>") + tr("Message decryption failed.") + QString("</nobr>"));
    //     return;
    // }

    ui->statusLabel_DD->setStyleSheet("QLabel { color: green; }");
    ui->statusLabel_DD->setText(QString("<nobr>") + tr("Message decrypted.") + QString("</nobr>"));
}

void MessagePage::on_clearButton_DD_clicked()
{
    ui->addressIn_DD->clear();
    ui->signatureIn_DD->clear();
    ui->messageIn_DD->clear();
    ui->statusLabel_DD->clear();

    ui->addressIn_DD->setFocus();
}

bool MessagePage::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::FocusIn)
    {
        if (ui->tabWidget->currentIndex() == 0)
        {
            /* Clear status message on focus change */
            ui->statusLabel_SM->clear();

            /* Select generated signature */
            if (object == ui->signatureOut_SM)
            {
                ui->signatureOut_SM->selectAll();
                return true;
            }
        }
        else if (ui->tabWidget->currentIndex() == 1)
        {
            /* Clear status message on focus change */
            ui->statusLabel_VM->clear();
        }

        else if (ui->tabWidget->currentIndex() == 2)
        {
            /* Clear status message on focus change */
            ui->statusLabel_ED->clear();

            /* Select generated signature */
            if (object == ui->signatureOut_ED)
            {
                ui->signatureOut_ED->selectAll();
                return true;
            }
        }
        else if (ui->tabWidget->currentIndex() == 3)
        {
            /* Clear status message on focus change */
            ui->statusLabel_DD->clear();
        }
    }
    return QDialog::eventFilter(object, event);
}
