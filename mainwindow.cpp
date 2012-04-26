#include <QLabel>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionLogin, SIGNAL(triggered()), this, SLOT(startLogin()));

    QLabel* permanentWidget = new QLabel(this);
    permanentWidget->setText("Based on Freebase API: http://www.freebase.com");
    statusBar()->addPermanentWidget(permanentWidget);
    m_pStatusLabel = new QLabel(this);
    statusBar()->addWidget(m_pStatusLabel);

    m_pForm = new Form(this);
    setCentralWidget(m_pForm);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startLogin()
{
    m_pForm->startLogin(true);
}

void MainWindow::startLoginDefault()
{
    m_pForm->startLogin(false);
}

void MainWindow::showStatusText(const QString& msg)
{
    m_pStatusLabel->setText(msg);
}