#include "resizinglabel.h"

#include <QPixmap>

ResizingLabel::ResizingLabel(QWidget *parent) :
  QLabel(parent)
{
}

void ResizingLabel::resizeEvent(QResizeEvent *)
{
  QSize scaledSize = m_originalPixmap.size();
  scaledSize.scale(size(), Qt::KeepAspectRatio);
  if (!pixmap() || scaledSize != pixmap()->size())
    adjustPixmapSize();
}

void ResizingLabel::adjustPixmapSize()
{
  QLabel::setPixmap(m_originalPixmap.scaled(size(), Qt::KeepAspectRatio));
  setMinimumSize(100, 100);
}

void ResizingLabel::setPixmap(const QPixmap &pixmap)
{
  m_originalPixmap = pixmap;
  adjustPixmapSize();
}
