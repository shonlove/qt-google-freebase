#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDate>
#include <QDesktopServices>

#include "dlgdemocountry.h"
#include "ui_dlgdemocountry.h"
#include "freebase_data_manager.h"

DlgDemoCountry::DlgDemoCountry(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgDemoCountry)
{
    ui->setupUi(this);

    ui->comboBoxLimit->setCurrentIndex(3);
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    m_pManager = new freebase_data_manager(this);

    connect(ui->pushButtonGoDemo,SIGNAL(clicked()),this,SLOT(startSearch()));
    connect(ui->comboBoxDemo,SIGNAL(currentIndexChanged(QString)),this,SLOT(onItemSelected()));
    connect(m_pManager,SIGNAL(sigJsonReady(int)),this,SLOT(onJsonReady(int)));

    connect(ui->webView->page()->networkAccessManager(),
              SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),
              this,
              SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> & )));
    connect(ui->webView,SIGNAL(linkClicked(QUrl)),this,SLOT(onLinkClicked(QUrl)));
}

DlgDemoCountry::~DlgDemoCountry()
{
    delete m_pManager;
    delete ui;
}

void DlgDemoCountry::startSearch()
{
    qDebug() << Q_FUNC_INFO;
    QString query = ui->lineEditInputDemo->text();
    if (!query.isEmpty()) {
        clearOldData();
        m_pManager->runSearchQuery(query+"&type=/location/country",ui->comboBoxLimit->currentText(),0);
    }
}

void DlgDemoCountry::onJsonReady(const int rt)
{
    qDebug() << Q_FUNC_INFO << " rt=" << rt;
    if (rt == REQ_SEARCH) {
        m_mapMids.clear();
        ui->comboBoxDemo->clear();
        QString mapKey;
        QVariantList list = m_pManager->getJsonData().toMap().contains("result") ?
                    m_pManager->getJsonData().toMap()["result"].toList() : QVariantList();

        QStringList names;
        foreach (QVariant item, list) {
            mapKey = "";
            QVariantMap map = item.toMap();
            if (map.contains("name")) {
                mapKey += map["name"].toString();
            }
            if (map.contains("notable")) {
                mapKey += map["notable"].toMap().contains("name") ? " (" + map["notable"].toMap()["name"].toString() + ")" : "";
            }
            if (map.contains("mid")) {
                m_mapMids[mapKey] = map["mid"].toString();
                names << mapKey;
            }
        }
        ui->comboBoxDemo->addItems(names);
        ui->labelCount->setText(m_pManager->getJsonData().toMap().contains("hits") ?
                                    m_pManager->getJsonData().toMap()["hits"].toString() : "No Items"
                                    );
    }
    else if (rt == REQ_MQL)
    {
        qDebug() << "Some info from MQL arrived";
//        QVariantMap map = m_pManager->getJsonData().toMap()["q0"].toMap()["result"].toMap();
//        QString strHtml = createHtmlForPerson(map);
        QString strHtml = createHtmlForPerson(m_pManager->getJsonData().toMap());
        ui->webView->setHtml(strHtml);
    }
}

void DlgDemoCountry::onItemSelected()
{
    QString str = ui->comboBoxDemo->currentText();
    qDebug() << Q_FUNC_INFO << " str=" << str;
    if (str != "Select item" && !str.isEmpty())
    {
        getCountryInfo(m_mapMids[str]);
    }
}

void DlgDemoCountry::getImage(const QString& mid)
{
    //int height = ui->graphicsViewDemo->size().height();
    //int width = ui->graphicsViewDemo->size().width();
    //clearReplyImage();
    //m_pManager->runImageQuery(mid,height,width);
}

void DlgDemoCountry::getCountryInfo(const QString& id)
{
    QStringList list;
    QString query = "{";
    if (id.startsWith("/m/"))
    {
        query += "\"mid\":\"" + id + "\"";
    }
    else
    {
        query += "\"id\":\"" + id + "\"";
    }
    QString type = "/location/country";
    query += QString(",\"type\":\"%1\", \"key\":[{}], \"*\":null}").arg(type);
    qDebug() << "QUERY: " << query;
    list << query;

    query = "{";
    if (id.startsWith("/m/")) {
        query += "\"mid\":\"" + id + "\"";
    } else {
        query += "\"id\":\"" + id + "\"";
    }
    type = "/location/location";
    query += QString(",\"type\":\"%1\", \"key\":[{}], \"*\":null}").arg(type);
    qDebug() << "QUERY: " << query;
    list << query;

    query = "{";
    if (id.startsWith("/m/")) {
        query += "\"mid\":\"" + id + "\"";
    } else {
        query += "\"id\":\"" + id + "\"";
    }
    type = "/location/region";
    query += QString(",\"type\":\"%1\", \"key\":[{}], \"*\":null}").arg(type);
    qDebug() << "QUERY: " << query;
    list << query;

    m_pManager->runMqlQueryMultiple(list);
}



void DlgDemoCountry::clearOldData()
{
    ui->labelCount->clear();
    //ui->labelName->setText("...");
    ui->comboBoxDemo->clear();
    //clearReplyImage();
}

QString DlgDemoCountry::createHtmlForPerson(const QVariantMap& map)
{
    QString strHtml = QString("<html><body>");
    QVariantList lst;
    strHtml += QString("<img src=\"https://usercontent.googleapis.com/freebase/v1-sandbox/image%1?maxheight=400&maxwidth=200\" align=\"right\">")
            .arg(map["q0"].toMap()["result"].toMap()["mid"].toString());

    // Referencies
    QString s = findNamespaceValue("/wikipedia/en_id",map);
    if (!s.isEmpty()) {
        strHtml += QString("<a href=\"http://www.freebase.com/view%1\"><img src=\"http://www.freebase.com/favicon.ico\" alt=\"Freebase\" hspace=\"2\"/>Freebase</a>")
                .arg(map["q0"].toMap()["result"].toMap()["mid"].toString());
        strHtml += QString("<a href=\"http://en.wikipedia.org/wiki/index.html?curid=%1\"><img src=\"http://en.wikipedia.org/favicon.ico\" alt=\"Wiki\" hspace=\"2\"/>Wiki</a>")
                .arg(s);
    }

    // Name
    strHtml += "<p><u>Name</u>: <b>" + map["q0"].toMap()["result"].toMap()["name"].toString() + "</b></p>";
    if(!map["q0"].toMap()["result"].toMap()["date_of_birth"].toString().isEmpty()) {
        QDate date = QDate::fromString(map["q0"].toMap()["result"].toMap()["date_of_birth"].toString(), Qt::ISODate);
        strHtml += "<p><u>Date of birth</u>: " + date.toString("MMM d, yyyy");

        if(!map["q1"].toMap()["result"].toMap()["date_of_death"].toString().isEmpty()) {
            QDate date = QDate::fromString(map["q1"].toMap()["result"].toMap()["date_of_death"].toString(), Qt::ISODate);
            strHtml += "<p><u>Date of depth</u>: " + date.toString("MMM d, yyyy");
        }
    }

    //Deceased person
    if(!map["q1"].toMap()["result"].toMap()["place_of_death"].toString().isEmpty())
    {
        strHtml += "<p><i>Deceased Person:</i></p>";
        strHtml += "<p><u>Place of death</u>: <b>" + map["q1"].toMap()["result"].toMap()["place_of_death"].toString() + "</b></p>";
    }
    lst = map["q1"].toMap()["result"].toMap()["place_of_burial"].toList();
    if(!lst.isEmpty())
    {
        strHtml += "<p><u>Place of burial</u>: <b>" + lst[0].toString() + "</b></p>";
    }
    lst = map["q1"].toMap()["result"].toMap()["cause_of_death"].toList();
    if(!lst.isEmpty())
    {
        QString str;
        for(int i = 0; i < lst.count(); ++i)
        {
            str += lst[i].toString();
            if(i < lst.count()-1)
                str += ", ";
        }
        strHtml += "<p><u>Cause of death</u>: <b>" + str + "</b></p>";
    }

    //Living person
    if(!map["q0"].toMap()["result"].toMap()["place_of_birth"].toString().isEmpty())
    {
        strHtml += "<p><i>Living Person:</i></p>";
        strHtml += "<p><u>Place of birth</u>: <b>" + map["q0"].toMap()["result"].toMap()["place_of_birth"].toString() + "</b></p>";
    }
    lst = map["q0"].toMap()["result"].toMap()["profession"].toList();
    if(!lst.isEmpty())
    {
        QString str;
        for(int i = 0; i < lst.count(); ++i)
        {
            str += lst[i].toString();
            if(i < lst.count()-1)
                str += ", ";
        }
        strHtml += "<p><u>Profession</u>: <b>" + str + "</b></p>";
    }
    lst = map["q0"].toMap()["result"].toMap()["nationality"].toList();
    if(!lst.isEmpty())
    {
        strHtml += "<p><u>Nationality</u>: <b>" + lst[0].toString() + "</b></p>";
    }
    lst = map["q0"].toMap()["result"].toMap()["ethnicity"].toList();
    if(!lst.isEmpty())
    {
        QString str;
        for(int i = 0; i < lst.count(); ++i)
        {
            str += lst[i].toString();
            if(i < lst.count()-1)
                str += ", ";
        }
        strHtml += "<p><u>Ethnicity</u>: <b>" + str + "</b></p>";
    }
    lst = map["q0"].toMap()["result"].toMap()["religion"].toList();
    if(!lst.isEmpty())
    {
        strHtml += "<p><u>Religion</u>: <b>" + lst[0].toString() + "</b></p>";
    }

    lst = map["q0"].toMap()["result"].toMap()["children"].toList();
    if(!lst.isEmpty())
    {
        QString children;
        for(int i = 0; i < lst.count(); ++i)
        {
            children += lst[i].toString();
            if(i < lst.count()-1)
                children += ", ";
        }
        strHtml += "<p><u>Children</u>: <b>" + children + "</b></p>";
    }

    lst = map["q0"].toMap()["result"].toMap()["parents"].toList();
    if(!lst.isEmpty())
    {
        QString parents;
        for(int i = 0; i < lst.count(); ++i)
        {
            parents += lst[i].toString();
            if(i < lst.count()-1)
                parents += ", ";
        }
        strHtml += "<p><u>Parents</u>: <b>" + parents + "</b></p>";
    }

    lst = map["q0"].toMap()["result"].toMap()["quotations"].toList();
    if(!lst.isEmpty())
    {
        QString str = "<ul>";
        for(int i = 0; i < lst.count(); ++i)
        {
            str += "<li>" + lst[i].toString() + "</li>";
        }
        str += "</ul>";
        strHtml += "<p><u>Quotations</u>:<br>" + str;
    }

    //Books author
//    strHtml += "<font color=\"green\">";
    lst = map["q2"].toMap()["result"].toMap()["works_written"].toList();
    if(!lst.isEmpty())
    {
        strHtml += "<p><i>Books author:</i></p>";
        QString str = "<ul>";
        for(int i = 0; i < lst.count(); ++i)
        {
            str += "<li>" + lst[i].toString() + "</li>";
        }
        str += "</ul>";
        strHtml += "<p><u>Works written</u>:<br>" + str + "</p>";
    }
//    strHtml += "</font>";

    strHtml += "</body></html>";
    return strHtml;
}

void DlgDemoCountry::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist)
{

  #if DEBUG_BUYIT
  qDebug() << "---frmBuyIt::sslErrorHandler: ";
  // show list of all ssl errors
  foreach (QSslError err, errlist)
    qDebug() << "ssl error: " << err;
  #endif

   qnr->ignoreSslErrors();
}

QString DlgDemoCountry::findNamespaceValue(const QString& ns, const QVariantMap& map)
{
    QString ret;
    QVariantList list = map["q0"].toMap()["result"].toMap()["key"].toList();
    foreach (QVariant item, list) {
        QVariantMap m = item.toMap();
        if (item.toMap()["namespace"].toString() == ns) {
            ret = item.toMap()["value"].toString();
            break;
        }
    }

    return ret;
}

void DlgDemoCountry::onLinkClicked(const QUrl& url)
{
    qDebug() << Q_FUNC_INFO << " url=" << url;
    QDesktopServices::openUrl(url);
}