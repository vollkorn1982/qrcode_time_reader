#ifndef QRCODE_TIME_READER_H
#define QRCODE_TIME_READER_H

#include <QMainWindow>
#include <QDateTime>

class QTableWidget;
class ResizingLabel;
class QLabel;

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
  
private:
  uint displayExif(const QString &filename, bool &bOk);
  uint readQRCode(const QString &filename, bool &bOk);

private:
  Ui::qrcode_time_reader *ui;
  QStringList             m_files; //!< List of all read filenames
  QTableWidget           *m_photoTable;
  ResizingLabel          *m_preview;
  QLabel                 *m_exifLabel;
  QLabel                 *m_qrLabel;
  QLabel                 *m_timeDifference;
  QDateTime               m_currentPicExifDateTime;
  QDateTime               m_currentPicQRDateTime;
};

#endif // QRCODE_TIME_READER_H
