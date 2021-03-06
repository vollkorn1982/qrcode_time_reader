#ifndef QRCODE_TIME_READER_H
#define QRCODE_TIME_READER_H

#include <QMainWindow>
#include <QDateTime>

class QTableWidget;
class ResizingLabel;
class QLabel;
class QPushButton;

namespace Ui {
  class qrcode_time_reader;
}

class qrcode_time_reader : public QMainWindow
{
  Q_OBJECT
  
public:
  explicit qrcode_time_reader(QWidget *parent = 0);
  ~qrcode_time_reader();

protected slots:
  void openFiles();
  void previewCurrentPhoto(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void adjustAllTimes();
  
private:
  struct PictureInfo {
    QString filename;
    uint exifTime;
    uint qrTime;
  };

private:
  uint readExifTime(const QString &filename, bool &bOk);
  uint readQRCodeTime(const QString &filename, bool &bOk);

private:
  Ui::qrcode_time_reader *ui;
  QList<PictureInfo>      m_pictures; //!< List of all read filenames
  int                     m_timeDifference;
  QTableWidget           *m_photoTable;
  ResizingLabel          *m_preview;
  QLabel                 *m_exifLabel;
  QLabel                 *m_qrLabel;
  QLabel                 *m_timeDifferenceLabel;
  QDateTime               m_currentPicExifDateTime;
  QDateTime               m_currentPicQRDateTime;
  QPushButton            *m_fixTimeButton;
};

#endif // QRCODE_TIME_READER_H
