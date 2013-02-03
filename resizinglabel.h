#ifndef RESIZINGLABEL_H
#define RESIZINGLABEL_H

#include <QLabel>

class QPixmap;

class ResizingLabel : public QLabel
{
  Q_OBJECT

public:
  explicit ResizingLabel(QWidget *parent = 0);
  void setPixmap(const QPixmap &pixmap);

protected:
  void resizeEvent(QResizeEvent *);
  void adjustPixmapSize();

private:
  QPixmap m_originalPixmap;
};

#endif // RESIZINGLABEL_H
