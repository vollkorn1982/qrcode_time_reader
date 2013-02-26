#include "qrcode_time_reader.h"
#include "ui_qrcode_time_reader.h"

#include <iomanip>
#include <iostream>

#include <exiv2/exiv2.hpp>
#include <opencv/highgui.h>
#include <decodeqr.h>

#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPixmap>
#include <QDateTime>
#include <QFile>
#include <QDir>

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
  m_qrLabel = findChild<QLabel*>("qrTime_data");
  m_timeDifference = findChild<QLabel*>("timeDiff_data");

  m_currentPicQRDateTime.setTimeSpec(Qt::UTC);
  m_currentPicExifDateTime.setTimeSpec(Qt::UTC);
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

  bool exifOk = false;
  uint exifTime = displayExif(m_files[currentRow], exifOk);
  if (exifOk)
    {
      m_currentPicExifDateTime.setTime_t(exifTime);
      m_exifLabel->setText(m_currentPicExifDateTime.toString() + " UTC");
    }
  else
    m_exifLabel->setText(tr("<no data>"));

  bool qrOk = false;
  uint qrTime = readQRCode(m_files[currentRow], qrOk);
  if (qrOk)
    {
      m_currentPicQRDateTime.setTime_t(qrTime);
      m_qrLabel->setText(m_currentPicQRDateTime.toString() + " UTC");
    }
  else
    m_qrLabel->setText(tr("<no data>"));

  if (qrOk && exifOk)
    {
      int difference = exifTime - qrTime;
      m_timeDifference->setText(QString::number(difference) + tr(" seconds"));
    }
  else
    m_timeDifference->setText(tr("<no data>"));
}

//! \brief Read EXIF time header and display it.
//! @param[in] filename Path and filename of the picture to display
//! @return time in seconds since 01.01.1970, -1 if invalid
uint qrcode_time_reader::displayExif(const QString &filename, bool &bOk)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    {
      bOk = false;
      return 0;
    }

  QByteArray imageData(file.readAll());
  Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((unsigned char *)imageData.data(), imageData.count());
  Q_ASSERT(image.get() != 0);
  image->readMetadata();

  Exiv2::ExifData &exifData = image->exifData();
  if (exifData.empty()) {
      bOk = false;
      return 0;
    }
  else {
      Exiv2::Exifdatum exivDatum(exifData["Exif.Photo.DateTimeOriginal"]);
      QDateTime time = QDateTime::fromString(QString::fromStdString(exivDatum.toString()), "yyyy:MM:dd hh:mm:ss");
      bOk = true;
      return time.toTime_t();
    }
}

//! \brief Read QR code time and display it.
//! @param[in] filename Path and filename of the picture to display
//! @param[out] bOk false wenn keine Konvertierung möglich ist
//! @return time in seconds since 01.01.1970, -1 if invalid
uint qrcode_time_reader::readQRCode(const QString &filename, bool &bOk)
{
  QImage resizeImage(filename);
  resizeImage = resizeImage.scaledToWidth(640, Qt::SmoothTransformation);
  resizeImage.save(filename + ".tmp", "JPG");

  IplImage *src = cvLoadImage(QFile::encodeName(filename + ".tmp").data());

  QFile file(filename + ".tmp");
  file.remove();

  if (!src)
    {
      bOk = false;
      return 0;
    }

  QrDecoderHandle decoder = qr_decoder_open();
  qr_decoder_decode_image(decoder,src);

  uint time;

  QrCodeHeader header;
  if (qr_decoder_get_header(decoder, &header)) {
      // get QR code text
      // To null terminate, a buffer size is larger than body size.
      char *buf = new char[header.byte_size + 1];
      qr_decoder_get_body(decoder, (unsigned char *)buf, header.byte_size + 1);

      time = QString(buf).toUInt();
      bOk = true;
    }
  else {
      bOk = false;
    }

  qr_decoder_close(decoder);
  cvReleaseImage(&src);
  return time;
}
