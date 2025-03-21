//
// Created by RGAA on 2023/8/19.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H
#define SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include <QPaintEvent>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include "db/stream_item.h"

namespace tc
{

    class StreamItemWidget : public QWidget {
    public:

        explicit StreamItemWidget(const StreamItem& item,  int bg_color, QWidget* parent = nullptr);
        ~StreamItemWidget() override;

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private:
        StreamItem item_;
        int bg_color_ = 0;
        QPixmap icon_;
        QPixmap bg_pixmap_;
        bool enter_ = false;
        QBitmap mask_;
        int radius_ = 10;
        QPushButton* btn_conn_ = nullptr;
        QWidget* btn_option_ = nullptr;
    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H
