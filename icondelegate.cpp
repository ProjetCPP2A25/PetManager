#include "icondelegate.h"
IconDelegate::IconDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
void IconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
}