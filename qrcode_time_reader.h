#ifndef QRCODE_TIME_READER_H
#define QRCODE_TIME_READER_H

#include <QMainWindow>

class QTableWidget;

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
  
private:
  Ui::qrcode_time_reader *ui;
  QString                 m_openFilesPath;
  QStringList             m_files;
  QTableWidget           *m_photoTable;
};

#endif // QRCODE_TIME_READER_H
