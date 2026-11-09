#ifndef ICONDELEGATE_H
#define ICONDELEGATE_H
#include <QStyledItemDelegate>
class IconDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit IconDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
#endif