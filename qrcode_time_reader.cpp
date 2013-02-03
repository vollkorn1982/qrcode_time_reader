#include "qrcode_time_reader.h"
#include "ui_qrcode_time_reader.h"

#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPixmap>
#include "resizinglabel.h"

#include <QDebug>

qrcode_time_reader::qrcode_time_reader(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::qrcode_time_reader)
{
  ui->setupUi(this);

  QAction *openAct(findChild<QAction*>("actionImport_photos"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(openFiles()));
  openAct->setShortcut(QKeySequence("Ctrl+o"));

  m_photoTable = findChild<QTableWidget*>("photoList");
  connect(m_photoTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(previewCurrentPhoto(int,int,int,int)));

  m_preview = findChild<ResizingLabel*>("photoPreview");
}

qrcode_time_reader::~qrcode_time_reader()
{
  delete ui;
}

void qrcode_time_reader::openFiles()
{
  m_files = QFileDialog::getOpenFileNames(
        NULL, NULL,
        "",
        tr("Photos (*.jpg)"),
        NULL,
        NULL);

  m_photoTable->setRowCount(m_files.count());
  for (int i = 0; i < m_files.count(); ++i)
    {
      QString filename = m_files[i];
      QTableWidgetItem *name = new QTableWidgetItem(filename.remove(0, m_files[i].lastIndexOf("/") + 1));
      name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_photoTable->setItem(i, 0, name);

      QTableWidgetItem *status = new QTableWidgetItem("<reading>");
      status->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_photoTable->setItem(i, 1, status);
    }
}

void qrcode_time_reader::previewCurrentPhoto(int currentRow, int /* currentColumn */, int /* previousRow */, int /* previousColumn */)
{
  m_preview->setPixmap(QPixmap(m_files[currentRow]));
}
