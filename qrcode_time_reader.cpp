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
#include <QPushButton>

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
  m_timeDifferenceLabel = findChild<QLabel*>("timeDiff_data");

  m_currentPicQRDateTime.setTimeSpec(Qt::UTC);
  m_currentPicExifDateTime.setTimeSpec(Qt::UTC);

  m_timeDifference = 0; //!< 0 means no time difference->fix time button disabled

  m_fixTimeButton = findChild<QPushButton*>("fixTimes");
  connect(m_fixTimeButton, SIGNAL(clicked()), this, SLOT(adjustAllTimes()));
}

qrcode_time_reader::~qrcode_time_reader()
{
  delete ui;
}

//! \brief Opens a file dialog for the user to select files to load.
void qrcode_time_reader::openFiles()
{
  QStringList filenames = QFileDialog::getOpenFileNames(
        NULL, NULL,
        "",
        tr("Photos (*.jpg *.jpeg)"),
        NULL,
        NULL);

  m_photoTable->setRowCount(filenames.count());
  for (int i = 0; i < filenames.count(); ++i) {
      PictureInfo picInfo;
      picInfo.filename = filenames[i];

      QTableWidgetItem *name = new QTableWidgetItem(filenames[i].remove(0, filenames[i].lastIndexOf("/") + 1));
      name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      m_photoTable->setItem(i, 0, name);

      bool bOk = false;
      picInfo.exifTime = readExifTime(picInfo.filename, bOk);
      picInfo.qrTime = readQRCodeTime(picInfo.filename, bOk);

      QTableWidgetItem *status = new QTableWidgetItem(tr("<reading>"));
      status->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

      if (bOk)
        {
          QDateTime time;
          time.setTime_t(picInfo.qrTime);
          status->setText(time.toString());
        }

      m_photoTable->setItem(i, 1, status);
      m_pictures.append(picInfo);
    }
}

//! \brief Displays the times and their difference in the preview
//! @param[in] currentRow the row corresponds to the index in \a m_files
void qrcode_time_reader::previewCurrentPhoto(int currentRow, int /* currentColumn */, int /* previousRow */, int /* previousColumn */)
{
  m_preview->setPixmap(QPixmap(m_pictures[currentRow].filename));
  qDebug() << QImage(m_pictures[currentRow].filename).text();

  bool exifOk = false;
  uint exifTime = readExifTime(m_pictures[currentRow].filename, exifOk);
  if (exifOk) {
      m_currentPicExifDateTime.setTime_t(exifTime);
      m_exifLabel->setText(m_currentPicExifDateTime.toString() + " UTC");
    }
  else
    m_exifLabel->setText(tr("<no data>"));

  bool qrOk = false;
  uint qrTime = readQRCodeTime(m_pictures[currentRow].filename, qrOk);
  if (qrOk) {
      m_currentPicQRDateTime.setTime_t(qrTime);
      m_qrLabel->setText(m_currentPicQRDateTime.toString() + " UTC");
    }
  else
    m_qrLabel->setText(tr("<no data>"));

  if (qrOk && exifOk) {
      m_timeDifference = exifTime - qrTime;
      m_fixTimeButton->setEnabled(m_timeDifference != 0);
      m_timeDifferenceLabel->setText(QString::number(m_timeDifference) + tr(" seconds"));
    }
  else
    {
      m_timeDifferenceLabel->setText(tr("<no data>"));
      m_fixTimeButton->setEnabled(false);
    }
}

//! \brief Read EXIF time header.
//! @param[in] filename Path and filename of the picture to display
//! @param[out] bOk false if no time could be read
//! @return time in seconds since 01.01.1970, 0 if no time was found
uint qrcode_time_reader::readExifTime(const QString &filename, bool &bOk)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
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

//! \brief Read QR code time.
//! @param[in] filename Path and filename of the picture to display
//! @param[out] bOk false if no time could be read
//! @return time in seconds since 01.01.1970, undefined if no time was found
uint qrcode_time_reader::readQRCodeTime(const QString &filename, bool &bOk)
{
  QImage resizeImage(filename);
  resizeImage = resizeImage.scaledToWidth(640, Qt::SmoothTransformation);
  resizeImage.save(filename + ".tmp", "JPG");

  IplImage *src = cvLoadImage(QFile::encodeName(filename + ".tmp").data());

  QFile file(filename + ".tmp");
  file.remove();

  if (!src) {
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

//! \brief Applies \a m_timeDifference to the EXIF time of all images in \a m_pictures
//! and writes the new value to the files.
void qrcode_time_reader::adjustAllTimes()
{
  for (int i = 0; i < m_pictures.size(); ++i) {
      m_pictures[i].exifTime -= m_timeDifference;
      Exiv2::Value::AutoPtr timeStr = Exiv2::Value::create(Exiv2::asciiString);

      // Set the value to a string
      QDateTime dateTime;
      dateTime.setTime_t(m_pictures[i].exifTime);
      timeStr->read(dateTime.toString("yyyy:MM:dd hh:mm:ss").toStdString().c_str());

      // Add the value together with its key to the Exif data container
      Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
      Exiv2::ExifData exifData;
      exifData.add(key, timeStr.get());

      Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(QFile::encodeName(m_pictures[i].filename).data());
      Q_ASSERT(image.get() != 0);

      image->setExifData(exifData);
      image->writeMetadata();
    }

  m_timeDifference = 0;
  m_fixTimeButton->setEnabled(false);

  // reload the preview to reflect the changes
  previewCurrentPhoto(m_photoTable->currentIndex().row(), 0, 0, 0);
}
