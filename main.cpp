#include <QApplication>
#include "qrcode_time_reader.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  qrcode_time_reader w;
  w.show();
  
  return a.exec();
}
