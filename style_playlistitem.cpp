#include "style_playlistitem.h"


void style_playlistitem::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();

    QString title = index.data(Qt::DisplayRole).toString();
    QString info = index.data(Qt::UserRole).toString();

    QRect rect = option.rect;
    painter->setRenderHint(QPainter::Antialiasing);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(rect, QColor(0x4578f9));
    else
        painter->fillRect(rect, QColor(0x34352f));


    painter->setPen(Qt::white);
    QFont titleFont = painter->font();
    titleFont.setBold(true);
    painter->setFont(titleFont);
    painter->drawText(rect.adjusted(10, 5, -10, -20), title);

    painter->setPen(Qt::white);
    QFont infoFont = painter->font();
    infoFont.setBold(false);
    painter->setFont(infoFont);
    painter->drawText(rect.adjusted(10, 25, -10, -5), info);



    painter->restore();
}

QSize style_playlistitem::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    return QSize(200, 50);
}


