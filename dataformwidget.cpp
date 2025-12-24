#include "dataformwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDate>
#include <QMessageBox>

DataFormWidget::DataFormWidget(QWidget *parent)
    : QWidget(parent), currentId(-1)
{
    auto *layout = new QVBoxLayout(this);

    auto *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(new QLabel("Название:"));
    nameEdit = new QLineEdit;
    nameLayout->addWidget(nameEdit);

    auto *typeLayout = new QHBoxLayout;
    typeLayout->addWidget(new QLabel("Тип:"));
    typeCombo = new QComboBox;
    typeCombo->addItems({"DV SSL", "OV SSL", "EV SSL", "IPsec", "Self-signed"}); //варианты сертификатов
    typeLayout->addWidget(typeCombo);

    auto *dateLayout = new QHBoxLayout;
    dateLayout->addWidget(new QLabel("Дата:"));
    dateEdit = new QLineEdit;
    // маска ввода: только цифры + дефисы, строго дд-мм-гггг[web:199][web:219]
    dateEdit->setInputMask("00-00-0000");
    dateEdit->setPlaceholderText("дд-мм-гггг");
    dateLayout->addWidget(dateEdit);

    auto *descLayout = new QVBoxLayout;
    descLayout->addWidget(new QLabel("Описание:"));
    descriptionEdit = new QTextEdit;
    descLayout->addWidget(descriptionEdit);

    auto *buttonsLayout = new QHBoxLayout;
    saveBtn   = new QPushButton("Сохранить");
    cancelBtn = new QPushButton("Отмена");
    clearBtn  = new QPushButton("Очистить");
    buttonsLayout->addWidget(saveBtn);
    buttonsLayout->addWidget(cancelBtn);
    buttonsLayout->addWidget(clearBtn);

    layout->addLayout(nameLayout);
    layout->addLayout(typeLayout);
    layout->addLayout(dateLayout);
    layout->addLayout(descLayout);
    layout->addLayout(buttonsLayout);

    // сохранение с проверкой даты
    connect(saveBtn, &QPushButton::clicked, [this]() {
        QString dateStr = dateEdit->text();
        QDate d = QDate::fromString(dateStr, "dd-MM-yyyy"); // тот же формат

        if (!d.isValid()) {
            QMessageBox::warning(this,
                                "Неверная дата",
                                "Дата должна быть в формате дд-мм-гггг и быть существующей.");
            dateEdit->setFocus();
            return;
        }

        if (onSave)
            onSave(getCertificate());
    });

    connect(cancelBtn, &QPushButton::clicked, [this]() {
        if (onCancel) onCancel();
    });
    connect(clearBtn, &QPushButton::clicked, this, &DataFormWidget::clearForm);
}

void DataFormWidget::setCertificate(const Certificate &c) {
    currentId = c.id;
    nameEdit->setText(QString::fromStdString(c.name));
    typeCombo->setCurrentText(QString::fromStdString(c.type));
    dateEdit->setText(QString::fromStdString(c.date));
    descriptionEdit->setPlainText(QString::fromStdString(c.description));
}

Certificate DataFormWidget::getCertificate() const {
    Certificate c;
    c.id = currentId;
    c.name = nameEdit->text().toStdString();
    c.type = typeCombo->currentText().toStdString();
    c.date = dateEdit->text().toStdString();
    c.description = descriptionEdit->toPlainText().toStdString();
    return c;
}

void DataFormWidget::clearForm() {
    currentId = -1;
    nameEdit->clear();
    dateEdit->clear();
    descriptionEdit->clear();
    typeCombo->setCurrentIndex(0);
}
