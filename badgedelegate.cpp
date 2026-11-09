#include "badgedelegate.h"
#include <QPainter>
#include <QFontMetrics>
BadgeDelegate::BadgeDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
void BadgeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.isEmpty()) { QStyledItemDelegate::paint(painter, option, index); return; }
    QColor bg, fg; int col = index.column();
    if (col == 1) {
        if (text == "Alimentation") { bg = QColor("#E3F5E9"); fg = QColor("#1E7A3C"); }
        else if (text == "Médicaments") { bg = QColor("#FDE7E7"); fg = QColor("#C0392B"); }
        else if (text == "Équipement") { bg = QColor("#E6F2F1"); fg = QColor("#2A8C82"); }
        else { bg = QColor("#F0F0F0"); fg = QColor("#666666"); }
    } else if (col == 6) {
        if (text == "Actif") { bg = QColor("#E3F5E9"); fg = QColor("#1E7A3C"); text = "✓ " + text; }
        else if (text == "Suspendu") { bg = QColor("#FFF4E0"); fg = QColor("#B26A00"); }
        else { bg = QColor("#F0F0F0"); fg = QColor("#666666"); }
    } else if (col == 8) {
        int val = text.toInt();
        if (val >= 75) { bg = QColor("#E3F5E9"); fg = QColor("#1E7A3C"); }
        else if (val >= 50) { bg = QColor("#FFF4E0"); fg = QColor("#B26A00"); }
        else { bg = QColor("#FDE7E7"); fg = QColor("#C0392B"); }
        text = text + "/100";
    } else { QStyledItemDelegate::paint(painter, option, index); return; }
    painter->save();
    QFont f = option.font; f.setBold(true); f.setPixelSize(12); painter->setFont(f);
    QFontMetrics fm(f); int textWidth = fm.horizontalAdvance(text);
    QRect r = option.rect; r.setLeft(r.left() + 12); r.setWidth(textWidth + 20); r.setHeight(24);
    r.moveCenter(QPoint(r.center().x(), option.rect.center().y()));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(bg); painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(r, 10, 10);
    painter->setPen(fg); painter->drawText(r, Qt::AlignCenter, text);
    painter->restore();
}