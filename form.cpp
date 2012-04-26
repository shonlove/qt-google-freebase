#include <QSettings>
#include <QMessageBox>
#include <QStatusBar>

#include "form.h"
#include "ui_form.h"
#include "oauth2.h"
#include "mainwindow.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    ui->tabQuery->setCurrentIndex(0);

    m_pMain = (MainWindow*)parent;
    m_pMain->showStatusText("Uknown user email");
    m_pOAuth2 = new OAuth2(this);

    m_strCompanyName = "YOU_COMPANY_NAME_HERE";
    m_strAppName = "QtFreebase";

    // Load settings
    m_pSettings = new QSettings(m_strCompanyName,m_strAppName);
    m_pOAuth2->setAccessToken(m_pSettings->value("access_token").toString());
    m_pOAuth2->setRefreshToken(m_pSettings->value("refresh_token").toString());
    m_pOAuth2->setSettings(m_pSettings);

    connect(m_pOAuth2, SIGNAL(loginDone()), this, SLOT(onLoginDone()));
    connect(m_pOAuth2, SIGNAL(sigErrorOccured(QString)),this,SLOT(onErrorOccured(QString)));

    m_pManager = new freebase_data_manager(this);
    connect(m_pManager, SIGNAL(sigUserEmailReady()),this,SLOT(onUserEmailReady()));
    connect(m_pManager, SIGNAL(sigMqlReplyReady()),this,SLOT(onMqlReplyReady()));

    QList<int> sizes;
    sizes << 150;
    sizes << 400;
    ui->splitter->setSizes(sizes);

    connect(m_pManager, SIGNAL(sigErrorOccured(QString)),this,SLOT(onErrorOccured(QString)));
    connect(ui->btnRun,SIGNAL(clicked()),this,SLOT(onBtnRunClicked()));
    connect(ui->btnClear,SIGNAL(clicked()),this,SLOT(onBtnClearClicked()));
}

Form::~Form()
{
    delete ui;
}

void Form::startLogin(bool bForce)
{
    //Now we allow to start logging in when m_oauth2.isAuthorized(). User can log in using another Google account.
    m_pOAuth2->startLogin(bForce); //this is a parent widget for a login dialog.
}

void Form::onErrorOccured(const QString& error)
{
    if(error.indexOf("Invalid Credentials") != -1) {
        startLogin(true);
    } else {
        QMessageBox::warning(this, tr("Some error on server occured"), error);
    }
}
/*! \brief Start function for initialization
 *
 */
void Form::onLoginDone()
{
    m_pManager->requestUserEmail(m_pOAuth2->accessToken());
}

void Form::saveSettings()
{
    m_pSettings->setValue("access_token",m_pOAuth2->accessToken());
    m_pSettings->setValue("refresh_token",m_pOAuth2->getRefreshToken());
}

void Form::onUserEmailReady()
{
    m_pMain->showStatusText(m_pManager->getUserEmail());
}

void Form::onMqlReplyReady()
{
    ui->textMqlReply->setPlainText(m_pManager->getReplyStr());
}

void Form::onBtnRunClicked()
{
    QString query = ui->editQuery->toPlainText();
//    query.chop(1);  //??? some additional character present: \20013 - remove it
    m_pManager->runQuery(query,m_pOAuth2->getSimpleAPIKey());
}

void Form::onBtnClearClicked()
{
    ui->textMqlReply->clear();
}