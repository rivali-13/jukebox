#ifndef STYLE_PLAYLISTITEM_H
#define STYLE_PLAYLISTITEM_H
#include <QStyledItemDelegate>
#include <QPainter>

//************* Change the appearance of playlist items **************
class style_playlistitem : public QStyledItemDelegate
{
public:
    style_playlistitem(QObject* parent = nullptr) : QStyledItemDelegate(parent) {} ;


    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override;
};
// ***
#endif // STYLE_PLAYLISTITEM_H
