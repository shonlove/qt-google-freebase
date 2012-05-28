#ifndef SIMPLESEARCHER_H
#define SIMPLESEARCHER_H

#include <QWidget>

#include <QVariantMap>
#include <QSslError>
#include <QUrl>

namespace Ui {
class SimpleSearcher;
}

class SimpleSearcher : public QWidget
{
    Q_OBJECT
    enum SearchSwitch { PREVIOUS, NEW, FORWARD };
    
public:
    explicit SimpleSearcher(QWidget *parent = 0);
    ~SimpleSearcher();

    void showTypeWidgets(bool);

protected slots:
    virtual void onLinkClicked(const QUrl &) {}

private slots:
    void onJsonReady(int);

    void on_findButton_clicked() { search(); }
    void on_addTypeButton_clicked();
    void on_backButton_clicked();
    void on_nextButton_clicked();
    void on_resultComboBox_activated(int index) { showPosition(index); }
    void on_prevButton_clicked() { search(PREVIOUS); }
    void on_forwButton_clicked() { search(FORWARD); }
    void on_searchLineEdit_returnPressed() { search(); }

    void sslErrorHandler(class QNetworkReply * qnr, const QList<QSslError> & /*errlist*/);

private:
    void search(SearchSwitch s = NEW);
    void showPosition(int pos);

protected:
    virtual QString getCurrentType() const;
    virtual QString createHtml(const QVariantMap & map);
    virtual void getInfo(const QString & id, const QString &type);
    virtual void delegatedRequest(const QVariantMap & ) {}

    void delegateRequests() { m_delegateMQLrequest = true; }
    class QWebView * webView() const;
    void appendToHistory(const QString & value, int pos = -1);

private:
    Ui::SimpleSearcher * ui;
    int m_currentResults;
    int m_resultsCount;

    int m_historyPos;
    bool m_delegateMQLrequest;

    // first: mid, second: result position in comboBox
    // second = -1 -> undefined position

    typedef QPair<QString, int> HistoryNode;
    QList<HistoryNode> m_history;

protected:
    class freebase_data_manager * m_pManager;
};

#endif // SIMPLESEARCHER_H
