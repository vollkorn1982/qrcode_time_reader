#include "qrcode_time_reader.h"
#include "ui_qrcode_time_reader.h"

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>

#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QDateTime>
#include <QFile>
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
  m_exifLabel = findChild<QLabel*>("exifTime_data");
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
        tr("Photos (*.jpg *.jpeg)"),
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
  qDebug() << QImage(m_files[currentRow]).text();

  displayExif(m_files[currentRow]);
}

void qrcode_time_reader::displayExif(const QString &filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return;

  QByteArray imageData(file.readAll());
  Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((unsigned char *)imageData.data(), imageData.count());
  Q_ASSERT(image.get() != 0);
  image->readMetadata();

  Exiv2::ExifData &exifData = image->exifData();
  if (exifData.empty()) {
      m_exifLabel->setText(tr("<no data>"));
    }
  else {
      Exiv2::Exifdatum exivDatum(exifData["Exif.Photo.DateTimeOriginal"]);
      m_currentPicExifDateTime = QDateTime::fromString(QString::fromStdString(exivDatum.toString()), "yyyy:MM:dd hh:mm:ss");
      m_exifLabel->setText(m_currentPicExifDateTime.toString());
    }
  return;
}
