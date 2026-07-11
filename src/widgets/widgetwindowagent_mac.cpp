// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#include "widgetwindowagent_p.h"

#include <QtGui/QtEvents>

namespace QWK {

    static inline QRect getWidgetSceneRect(QWidget *widget) {
        return widget ? QRect(widget->mapTo(widget->window(), QPoint()), widget->size()) : QRect();
    }

    class SystemButtonAreaWidgetEventFilter : public QObject {
    public:
        SystemButtonAreaWidgetEventFilter(QWidget *widget, AbstractWindowContext *ctx,
                                          QObject *parent = nullptr)
            : QObject(parent), widget(widget), ctx(ctx) {
            widget->installEventFilter(this);
            const QPointer<QWidget> guardedWidget(widget);
            ctx->setSystemButtonAreaCallback([guardedWidget](const QSize &) {
                return getWidgetSceneRect(guardedWidget.data());
            });
        }
        ~SystemButtonAreaWidgetEventFilter() override = default;

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override {
            Q_UNUSED(obj)
            switch (event->type()) {
                case QEvent::Move:
                case QEvent::Resize: {
                    ctx->virtual_hook(AbstractWindowContext::SystemButtonAreaChangedHook, nullptr);
                    break;
                }

                default:
                    break;
            }
            return false;
        }

    protected:
        QPointer<QWidget> widget;
        AbstractWindowContext *ctx;
    };

    bool WidgetWindowAgentPrivate::installPlatformSystemButtons() {
        return context != nullptr;
    }

    QRect WidgetWindowAgentPrivate::platformSystemButtonAreaGeometry() const {
        if (systemButtonAreaWidget) {
            return getWidgetSceneRect(systemButtonAreaWidget);
        }
        return systemButtonAreaRect;
    }

    /*!
        Returns the widget that acts as the system button area.
    */
    QWidget *WidgetWindowAgent::systemButtonArea() const {
        Q_D(const WidgetWindowAgent);
        return d->systemButtonAreaWidget;
    }

    /*!
        Sets the widget that positions the native traffic-light buttons. The buttons are centered
        in the widget's window-space geometry. Moving or resizing the widget updates their position.
    */
    void WidgetWindowAgent::setSystemButtonArea(QWidget *widget) {
        Q_D(WidgetWindowAgent);
        if (widget && d->systemButtonAreaWidget == widget)
            return;

        auto ctx = d->context.get();
        d->systemButtonAreaWidget = widget;
        d->systemButtonAreaRect = {};
        if (!widget) {
            d->context->setSystemButtonAreaCallback({});
            d->systemButtonAreaWidgetEventFilter.reset();
            return;
        }
        d->systemButtonAreaWidgetEventFilter =
            std::make_unique<SystemButtonAreaWidgetEventFilter>(widget, ctx);
    }

    /*!
        Sets the placement area for the native traffic-light buttons in top-level widget
        coordinates. AppKit owns and draws the buttons; QWindowKit centers them in \a rect.
    */
    void WidgetWindowAgent::setSystemButtonAreaGeometry(const QRect &rect) {
        Q_D(WidgetWindowAgent);
        if (!d->systemButtonAreaWidget && d->systemButtonAreaRect == rect) {
            return;
        }

        d->systemButtonAreaWidget = nullptr;
        d->systemButtonAreaWidgetEventFilter.reset();
        d->systemButtonAreaRect = rect;
        if (!rect.isValid()) {
            d->context->setSystemButtonAreaCallback({});
            return;
        }
        d->context->setSystemButtonAreaCallback([rect](const QSize &) {
            return rect;
        });
    }

    /*!
        Returns the system button area callback.
    */
    ScreenRectCallback WidgetWindowAgent::systemButtonAreaCallback() const {
        Q_D(const WidgetWindowAgent);
        return d->systemButtonAreaWidget ? nullptr : d->context->systemButtonAreaCallback();
    }

    /*!
        Sets the system button area callback. The \c size argument is the native title
        bar size.

        The system button position will be updated when the window resizes.
    */
    void WidgetWindowAgent::setSystemButtonAreaCallback(const ScreenRectCallback &callback) {
        Q_D(WidgetWindowAgent);
        setSystemButtonArea(nullptr);
        d->systemButtonAreaRect = {};
        d->context->setSystemButtonAreaCallback(callback);
    }

}
