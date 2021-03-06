/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qpropertybinding_p.h"
#include "qproperty_p.h"

#include <QScopedValueRollback>

QT_BEGIN_NAMESPACE

using namespace QtPrivate;

QPropertyBindingPrivate::~QPropertyBindingPrivate()
{
    if (firstObserver)
        firstObserver.unlink();
}

void QPropertyBindingPrivate::unlinkAndDeref()
{
    propertyDataPtr = nullptr;
    if (!ref.deref())
        delete this;
}

void QPropertyBindingPrivate::markDirtyAndNotifyObservers()
{
    dirty = true;
    if (firstObserver)
        firstObserver.notify(this);
}

bool QPropertyBindingPrivate::evaluateIfDirtyAndReturnTrueIfValueChanged()
{
    if (!dirty)
        return false;

    if (updating) {
        error = QPropertyBindingError(QPropertyBindingError::BindingLoop);
        return false;
    }

    QScopedValueRollback<bool> updateGuard(updating, true);

    BindingEvaluationState evaluationFrame(this);

    QUntypedPropertyBinding::BindingEvaluationResult result;
    bool changed = false;
    if (metaType.id() == QMetaType::Bool) {
        auto propertyPtr = reinterpret_cast<QPropertyBase *>(propertyDataPtr);
        bool temp = propertyPtr->extraBit();
        result = evaluationFunction(metaType, &temp);
        if (auto changedPtr = std::get_if<bool>(&result)) {
            changed = *changedPtr;
            if (changed)
                propertyPtr->setExtraBit(temp);
        }
    } else {
        result = evaluationFunction(metaType, propertyDataPtr);
        if (auto changedPtr = std::get_if<bool>(&result))
            changed = *changedPtr;
    }

    if (auto errorPtr = std::get_if<QPropertyBindingError>(&result))
        error = std::move(*errorPtr);

    dirty = false;
    return changed;
}

QUntypedPropertyBinding::QUntypedPropertyBinding() = default;

QUntypedPropertyBinding::QUntypedPropertyBinding(const QMetaType &metaType, QUntypedPropertyBinding::BindingEvaluationFunction function,
                                                 const QPropertyBindingSourceLocation &location)
{
    d = new QPropertyBindingPrivate(metaType, std::move(function), std::move(location));
}

QUntypedPropertyBinding::QUntypedPropertyBinding(QUntypedPropertyBinding &&other)
    : d(std::move(other.d))
{
}

QUntypedPropertyBinding::QUntypedPropertyBinding(const QUntypedPropertyBinding &other)
    : d(other.d)
{
}

QUntypedPropertyBinding &QUntypedPropertyBinding::operator=(const QUntypedPropertyBinding &other)
{
    d = other.d;
    return *this;
}

QUntypedPropertyBinding &QUntypedPropertyBinding::operator=(QUntypedPropertyBinding &&other)
{
    d = std::move(other.d);
    return *this;
}

QUntypedPropertyBinding::QUntypedPropertyBinding(const QPropertyBindingPrivatePtr &priv)
    : d(priv)
{
}

QUntypedPropertyBinding::~QUntypedPropertyBinding()
{
}

bool QUntypedPropertyBinding::isNull() const
{
    return !d;
}

QPropertyBindingError QUntypedPropertyBinding::error() const
{
    if (!d)
        return QPropertyBindingError();
    return d->error;
}

QMetaType QUntypedPropertyBinding::valueMetaType() const
{
    if (!d)
        return QMetaType();
    return d->metaType;
}

void QUntypedPropertyBinding::setDirty(bool dirty)
{
    if (!d)
        return;
    d->dirty = dirty;
}

QT_END_NAMESPACE
