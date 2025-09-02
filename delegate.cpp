/*
 * Copyright (C) 2018-2019 Factory #12
 * Copyright (C) 2020-2024 Armands Aleksejevs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

/*
 * includes
 */
#include "delegate.h"
#include "mainwindow.h"
#include "task.h"
#include <QDebug>
#include <QSpinBox>
#include <QStylePainter>

/**
 * @brief Delegate::paint
 * @param painter
 * @param option
 * @param index
 */
void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const Task::Types type = Task::instance()->type(row(index));
    const QFont font = Task::instance()->data(index, Qt::FontRole).value<QFont>();
    const int buttonSize = buttonSizes.isEmpty() ? 0 : buttonSizes[index];
    const int margin = 4;
    const QRect rect(option.rect.left() + margin, option.rect.top(), option.rect.width() - buttonSize - margin, Delegate::ItemHeight);
    const bool edit = currentEditIndex() == index;
    const Id comboId = combos.isEmpty() ? Id::Invalid : combos[index];
    const bool isComboActive = MainWindow::instance()->isComboModeActive();
    const bool isEventActive = MainWindow::instance()->currentEvent() != Row::Invalid;
    const bool isTeamActive = MainWindow::instance()->currentTeam() != Row::Invalid;

    if (!isTeamActive) return;

    // save state
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // draw alternating row colours
    if (index.row() & 1) painter->fillRect(option.rect, QColor::fromRgb(0, 0, 0, 16));

    if (option.state & QStyle::State_MouseOver) {
        painter->save();
        painter->setBrush(Qt::transparent);

        const int g = qGray(option.palette.highlight().color().rgb());

        painter->setPen({ QColor::fromRgb(g, g, g, 192), 2, Qt::DashLine });
        painter->drawRoundedRect(QRect(option.rect.x() + 1, option.rect.y() + 1, option.rect.width() - 2, option.rect.height() - 2), 10, 10);
        painter->restore();
    }

    // combo
    if (comboId != Id::Invalid) {
        if (!relativeCombos.contains(comboId)) relativeCombos[comboId] = ++lastRelativeCombo;
    }

    // abort if no active combos are visible
    if (isComboActive && !combos.values().contains(MainWindow::instance()->currentComboId())) MainWindow::instance()->setTaskFilter();

    // store rectSize
    rectSizes[index] = rect;

    // draw cross/equals lambda
    auto drawCrossEquals = [this, painter, index, rect, type, edit, isComboActive, isEventActive, isTeamActive]() {
        const int isSelected = edit ? false : (index == currentIndex());
        const bool hasValue = values.isEmpty() ? false : values[index] != 0;
        QRect small(rect.right() + Delegate::ButtonWidth * 2 + Delegate::SmallWidth, rect.top(), Delegate::SmallWidth, Delegate::ItemHeight);

        if (type == Task::Types::Check || isComboActive || !isEventActive || !isTeamActive) return;

        if ((hasValue && !isSelected) || (!hasValue && isSelected)) {
            painter->drawPixmap(small, Cross());
            painter->drawPixmap(small.translated(Delegate::ButtonWidth + Delegate::SmallWidth, 0), Equals());
        }
    };

    // draw items
    if ((currentEditIndex() == QModelIndex() || edit || isComboActive) && isEventActive && isTeamActive) {
        if (edit || (isComboActive && comboId != Id::Invalid)) {
            QColor highlight(option.palette.highlight().color());
            highlight.setAlpha(128);
            painter->fillRect(option.rect, highlight);
        }

        const QList<Item> items(getItems(index));
        for (const Item &item : items) item.paint(painter, index);

        drawCrossEquals();
    }

    // set up font and draw task name
    painter->setFont({ option.font.family(), static_cast<int>(Delegate::ItemHeight * 0.4), font.weight(), font.italic() });
    painter->drawText(rect, QFontMetrics(painter->font()).elidedText(Task::instance()->name(row(index)), Qt::ElideRight, rect.width()),
        { Qt::AlignLeft | Qt::AlignVCenter });

    // disable view
    if (currentEditIndex() != QModelIndex() && !edit && !isComboActive) {
        QColor foreground(option.palette.windowText().color());
        foreground.setAlpha(16);
        painter->fillRect(option.rect, foreground);
    }

    // restore state
    painter->restore();
}

/**
 * @brief Delegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size(QStyledItemDelegate::sizeHint(option, index));
    const Task::Types type = Task::instance()->type(row(index));
    const int buttonSize = MainWindow::instance()->isComboModeActive()
        ? Delegate::ButtonWidth
        : ((type == Task::Types::Multi) ? Delegate::ButtonWidth * 4 + Delegate::SmallWidth * 3 : Delegate::ButtonWidth * 3 + Delegate::SmallWidth * 2);
    const bool isTeamActive = MainWindow::instance()->currentTeam() != Row::Invalid;

    buttonSizes[index] = buttonSize;
    size.setWidth(buttonSize);
    size.setHeight(Delegate::ItemHeight);

    if (isTeamActive) {
        combos[index] = Task::instance()->comboId(row(index));
        values[index] = Task::instance()->multiplier(row(index));
    }

    return size;
}

/**
 * @brief Delegate::getItems
 * @return
 */
QList<Item> Delegate::getItems(const QModelIndex &index) const {
    const QRect rect(rectSizes.isEmpty() ? QRect() : rectSizes[index]);
    QRect button(rect.right(), rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight);
    const Item info(Item::Info, button, this);
    const Item multi(Item::Multi, button.translated(Delegate::ButtonWidth + Delegate::SmallWidth, 0), this);

    if (MainWindow::instance()->isComboModeActive()) return QList<Item>() << Item(Item::Multi, button, this);

    return Task::instance()->type(row(index)) == Task::Types::Multi
        ? QList<Item>() << info << multi << Item(Item::Numeric, button.translated(Delegate::ButtonWidth * 2 + Delegate::SmallWidth * 2, 0), this)
                        << Item(Item::Sum, button.translated((Delegate::ButtonWidth + Delegate::SmallWidth) * 3, 0), this)
        : QList<Item>() << info << multi << Item(Item::Checkable, button.translated(Delegate::ButtonWidth * 2 + Delegate::SmallWidth * 2, 0), this);
}

/**
 * @brief Delegate::action
 * @param index
 * @return
 */
Item::Actions Delegate::action(const QModelIndex &index) const {
    Item::Actions action = Item::NoAction;

    const QList<Item> items(getItems(index));
    for (const Item &item : items) {
        action = item.action(index);
        if (action != Item::NoAction) return action;
    }

    return action;
}

/**
 * @brief Delegate::setMousePos
 * @param pos
 */
void Delegate::setMousePos(const QPoint &pos, bool outside) {
    int y;

    // failsafe
    if (view() == nullptr || m_pos == pos) return;

    // store position
    m_pos = pos;

    // leave event
    if (outside) {
        view()->update(currentIndex());
        m_currentIndex = QModelIndex();
    }

    // search for a rect that contains mouse
    bool found = false;
    for (y = 0; y < Task::instance()->count(); y++) {
        const QModelIndex index(view()->model()->index(y, Task::instance()->Name));

        if (view()->visualRect(index).contains(pos)) {
            view()->update(currentIndex());
            m_currentIndex = index;
            view()->update(currentIndex());

            found = true;
            break;
        }
    }

    if (!found) {
        if (currentIndex().isValid() && view() != nullptr) view()->update(currentIndex());
        m_currentIndex = QModelIndex();
    }
}

/**
 * @brief Delegate::reset
 */
void Delegate::reset() {
    m_pos = QPoint();
    m_currentIndex = QModelIndex();
    m_currentEditIndex = QModelIndex();
    currentEditWidget = nullptr;

    // button sizes
    rectSizes.clear();
    buttonSizes.clear();
    values.clear();
    combos.clear();
    relativeCombos.clear();
    lastRelativeCombo = 0;
    m_value = 0;
}

/**
 * @brief Delegate::createEditor
 * @param parent
 * @return
 */
QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const {
    EditWidget *edit(new EditWidget(this, index, parent));

    // store current index
    m_currentEditIndex = index;
    currentEditWidget = edit;

    // set up widget
    edit->setMinimum(-9999);
    edit->setMaximum(Task::instance()->pattern(row(index)).isEmpty() ? Task::instance()->multi(row(index)) : 9999);
    edit->setAlignment(Qt::AlignCenter);
    edit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    edit->setStyleSheet("QSpinBox { background-color: transparent; color: white; text-align: center; selection-background-color: transparent; } "
                        "QSpinBox::up-button { width: 0px; } QSpinBox::down-button { width: 0px; }");

    // return a newly constructed editor widget
    return edit;
}

/**
 * @brief Delegate::setEditorData
 * @param editor
 * @param index
 */
void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    EditWidget *editWidget(qobject_cast<EditWidget *>(editor));
    const int value = values.isEmpty() ? 0 : values[index];
    editWidget->setValue(value);
    m_value = value;
}

/**
 * @brief Delegate::setModelData
 * @param editor
 * @param model
 * @param index
 */
void Delegate::setModelData(QWidget *editor, QAbstractItemModel *, const QModelIndex &index) const {
    EditWidget *editWidget(qobject_cast<EditWidget *>(editor));
    editWidget->interpretText();
    Task::instance()->setMultiplier(row(index), editWidget->value());
}

/**
 * @brief Delegate::updateEditorGeometry
 * @param editor
 * @param option
 */
void Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    editor->setGeometry({ rectSizes.isEmpty() ? 0 : rectSizes[index].right() + Delegate::ButtonWidth * 2 + Delegate::SmallWidth * 2,
        option.rect.top(), Delegate::ButtonWidth, Delegate::ItemHeight });
}

/**
 * @brief Delegate::destroyEditor
 * @param editor
 * @param index
 */
void Delegate::destroyEditor(QWidget *editor, const QModelIndex &index) const {
    // reset index and delete widget
    m_currentEditIndex = QModelIndex();
    currentEditWidget = nullptr;
    QStyledItemDelegate::destroyEditor(editor, index);
}

/**
 * @brief EditWidget::paintEvent
 * @param event
 */
void EditWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    const Item item(Item::Editor, QRect(0, 0, Delegate::ButtonWidth, Delegate::ItemHeight), delegate);

    // set font size determined in fontSizeForWidth
    // setFont( font );

    // paint background
    item.paint(&painter, index);

    // paint input
    QSpinBox::paintEvent(event);
}

/**
 * @brief EditWidget::validate
 * @param text
 * @param pos
 * @return
 */
QValidator::State EditWidget::validate(QString &text, int &) const {
    if (text.isEmpty()) return QValidator::Acceptable;

    bool ok;
    const int value = text.toInt(&ok);

    if (!ok) return QValidator::Intermediate;

    const Row row = Task::instance()->row(index);
    if (row == Row::Invalid) return QValidator::Invalid;
    if (Task::instance()->pattern(Task::instance()->row(index)).isEmpty()) return QValidator::Acceptable;

    return Task::instance()->validate(row, value) ? QValidator::Acceptable : QValidator::Intermediate;
}

/**
 * @brief Delegate::fontSizeForWidth
 * @param text
 * @param baseFont
 * @param width
 * @return
 */
QFont Delegate::fontSizeForWidth(const QString &text, const QFont &baseFont, qreal width) {
    QFont font(baseFont);

    // ignore invalid sizes
    if (width <= 4.0) return font;

    // determine optimal font size
    forever {
        const QFontMetricsF fm(font);

        if (font.pointSize() < 4) break;
        if (fm.horizontalAdvance(text) < static_cast<qreal>(width) * 0.90) break;

        font.setPointSizeF(font.pointSizeF() * 0.75);
    }

    return font;
}

/**
 * @brief Delegate::currentEditorValue
 * @return
 */
int Delegate::currentEditorValue() const {
    if (currentEditIndex() == QModelIndex() || currentEditWidget == nullptr) return 0;

    // get widget
    EditWidget *edit(qobject_cast<EditWidget *>(currentEditWidget));

    // get font
    QFont font(edit->font());
    font.setBold(true);
    font.setPointSizeF(edit->height() * 0.64);
    edit->setFont(Delegate::fontSizeForWidth(QString::number(edit->value()), font, edit->width()));

    // update item on change
    view()->update(currentEditIndex());

    // return value
    return edit->value();
}

/**
 * @brief Delegate::proxyRow
 * @param index
 * @return
 */
Row Delegate::row(const QModelIndex &index) const {
    const Table *table(qobject_cast<const Table *>(index.model()));

    if (table == nullptr) {
        qDebug() << tr("invalid model") << index.model()->objectName();
        return Row::Invalid;
    }

    return table->row(index);
}
