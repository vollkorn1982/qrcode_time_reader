#include "qrcode_time_reader.h"
#include "ui_qrcode_time_reader.h"

#include <QFileDialog>
#include <QTableWidget>

#include <QDebug>

qrcode_time_reader::qrcode_time_reader(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::qrcode_time_reader)
{
  ui->setupUi(this);

  QAction *openAct(findChild<QAction*>("actionImport_photos"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(openFiles()));

  m_photoTable = findChild<QTableWidget*>("photoList");
}

qrcode_time_reader::~qrcode_time_reader()
{
  delete ui;
}

void qrcode_time_reader::openFiles()
{
  QFileDialog::Options options;
  m_files = QFileDialog::getOpenFileNames(
        this, NULL,
        m_openFilesPath,
        tr("Photos (*.jpg)"),
        NULL,
        options);
}

