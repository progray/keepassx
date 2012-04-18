/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntryAttributes.h"

const QStringList EntryAttributes::DEFAULT_ATTRIBUTES(QStringList() << "Title" << "URL"
                                                      << "UserName" << "Password" << "Notes");

EntryAttributes::EntryAttributes(QObject* parent)
    : QObject(parent)
{
    clear();
}

QList<QString> EntryAttributes::keys() const
{
    return m_attributes.keys();
}

QString EntryAttributes::value(const QString& key) const
{
    return m_attributes.value(key);
}

bool EntryAttributes::isProtected(const QString& key) const
{
    return m_protectedAttributes.contains(key);
}

void EntryAttributes::set(const QString& key, const QString& value, bool protect)
{
    bool emitModified = false;

    bool addAttribute = !m_attributes.contains(key);
    bool defaultAttribute = isDefaultAttribute(key);

    if (addAttribute && !defaultAttribute) {
        Q_EMIT aboutToBeAdded(key);
    }

    if (addAttribute || m_attributes.value(key) != value) {
        m_attributes.insert(key, value);
        emitModified = true;
    }

    if (protect) {
        if (!m_protectedAttributes.contains(key)) {
            emitModified = true;
        }
        m_protectedAttributes.insert(key);
    }
    else if (m_protectedAttributes.remove(key)) {
        emitModified = true;
    }

    if (emitModified) {
        Q_EMIT modified();
    }

    if (defaultAttribute) {
        Q_EMIT defaultKeyModified();
    }
    else if (addAttribute) {
        Q_EMIT added(key);
    }
    else if (emitModified) {
        Q_EMIT customKeyModified(key);
    }
}

void EntryAttributes::remove(const QString& key)
{
    Q_ASSERT(!isDefaultAttribute(key));

    if (!m_attributes.contains(key)) {
        Q_ASSERT(false);
        return;
    }

    Q_EMIT aboutToBeRemoved(key);

    m_attributes.remove(key);
    m_protectedAttributes.remove(key);

    Q_EMIT removed(key);
    Q_EMIT modified();
}

void EntryAttributes::copyCustomKeysFrom(const EntryAttributes* other)
{
    if (!areCustomKeysDifferent(other)) {
        return;
    }

    Q_EMIT aboutToBeReset();

    // remove all non-default keys
    Q_FOREACH (const QString& key, keys()) {
        if (!isDefaultAttribute(key)) {
            m_attributes.remove(key);
            m_protectedAttributes.remove(key);
        }
    }

    Q_FOREACH (const QString& key, other->keys()) {
        if (!isDefaultAttribute(key)) {
            m_attributes.insert(key, other->value(key));
            if (other->isProtected(key)) {
                m_protectedAttributes.insert(key);
            }
        }
    }

    Q_EMIT reset();
    Q_EMIT modified();
}

bool EntryAttributes::areCustomKeysDifferent(const EntryAttributes* other)
{
    // check if they are equal ignoring the order of the keys
    if (keys().toSet() != other->keys().toSet()) {
        return true;
    }

    Q_FOREACH (const QString& key, keys()) {
        if (isDefaultAttribute(key)) {
            continue;
        }

        if (isProtected(key) != other->isProtected(key) || value(key) != other->value(key)) {
            return true;
        }
    }

    return false;
}

void EntryAttributes::clear()
{
    Q_EMIT aboutToBeReset();

    m_attributes.clear();
    m_protectedAttributes.clear();

    Q_FOREACH (const QString& key, DEFAULT_ATTRIBUTES) {
        m_attributes.insert(key, "");
    }

    Q_EMIT reset();
    Q_EMIT modified();
}

bool EntryAttributes::isDefaultAttribute(const QString& key)
{
    return DEFAULT_ATTRIBUTES.contains(key);
}